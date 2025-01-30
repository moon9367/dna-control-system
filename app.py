import os
import serial
from datetime import datetime
from flask import Flask, render_template, request, send_file, jsonify
from picamera2 import Picamera2
import zipfile

# Flask 설정
app = Flask(__name__)
picam2 = Picamera2()

# 카메라 설정
config = picam2.create_still_configuration(main={"size": (1920, 1080)})
picam2.configure(config)
picam2.start()

# 아두이노 시리얼 포트 자동 감지
def find_serial_port():
    possible_ports = ["/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyACM0", "/dev/ttyACM1"]
    for port in possible_ports:
        try:
            ser = serial.Serial(port, 9600, timeout=1)
            print(f"✅ Arduino 연결 성공: {port}")
            return ser
        except serial.SerialException:
            continue
    print("⚠️ Arduino 연결 실패: USB 포트 확인 필요")
    return None

ser = find_serial_port()

# 사진 저장 경로
PHOTO_FOLDER = "/home/aiseed/photos"
STATIC_PHOTO_PATH = "/home/aiseed/dna-control-system/static/photo.jpg"
latest_photo_path = None  # 최신 사진 경로 저장 변수

# 폴더가 없으면 생성
if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/set_temp", methods=["POST"])
def set_temp():
    """목표 온도 설정"""
    data = request.get_json()
    target_temp = data["temperature"]
    ser.write(f"set_temp:{target_temp}\n".encode())
    response = ser.readline().decode().strip()
    return jsonify({"message": f"온도 설정: {target_temp}°C", "response": response})

@app.route("/heater", methods=["POST"])
def heater_control():
    """히터 ON/OFF 제어"""
    data = request.get_json()
    action = data["action"].lower()
    ser.write(f"heater_{action}\n".encode())
    response = ser.readline().decode().strip()
    return jsonify({"message": f"Heater {action}", "response": response})

@app.route("/led", methods=["POST"])
def led_control():
    """LED ON/OFF 제어"""
    data = request.get_json()
    action = data["action"].lower()
    ser.write(f"led_{action}\n".encode())
    response = ser.readline().decode().strip()
    return jsonify({"message": f"LED {action}", "response": response})

@app.route("/temperature")
def get_temperature():
    """현재 온도, LED, 히터 상태 가져오기"""
    if not ser:
        return jsonify({"error": "시리얼 포트 연결 실패"}), 500

    ser.write("get_temp\n".encode())
    response = ser.readlines()
    temp, led, heater = "", "", ""

    for line in response:
        line = line.decode().strip()
        if line.startswith("temp:"):
            temp = str(int(float(line.split(":")[1])))  # 소수점 제거
        elif line.startswith("led:"):
            led = line.split(":")[1]
        elif line.startswith("heater:"):
            heater = line.split(":")[1]

    return jsonify({
        "temperature": temp,
        "led": led,
        "heater": heater,
    })

@app.route("/capture", methods=["POST"])
def capture_photo():
    """사진 촬영 및 최신 사진 저장"""
    global latest_photo_path
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest_photo_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")

    try:
        picam2.capture_file(latest_photo_path)
        print(f"사진 촬영 완료: {latest_photo_path}")

        # 최신 파일명을 JSON 응답으로 반환
        return jsonify({"message": "사진 촬영 완료", "photo_url": f"/photos/{os.path.basename(latest_photo_path)}"})

    except Exception as e:
        print(f"사진 촬영 오류: {e}")
        return jsonify({"error": "사진 촬영 실패"}), 500

@app.route("/download_current", methods=["GET"])
def download_current():
    """현재 최신 사진 다운로드"""
    if latest_photo_path is None or not os.path.exists(latest_photo_path):
        print(f"다운로드 오류: 파일이 존재하지 않음 → {latest_photo_path}")
        return "현재 다운로드할 사진이 없습니다.", 404

    print(f"다운로드 요청: {latest_photo_path}")
    return send_file(latest_photo_path, as_attachment=True)


@app.route("/download_all", methods=["GET"])
def download_all():
    """저장된 모든 사진을 ZIP 파일로 다운로드"""
    zip_path = os.path.join(PHOTO_FOLDER, "photos.zip")

    # 폴더 내 파일 확인
    photo_files = [f for f in os.listdir(PHOTO_FOLDER) if f.endswith(".jpg")]

    if not photo_files:
        print("다운로드 실패: 폴더 내 사진 없음")
        return "폴더에 저장된 사진이 없습니다.", 404

    try:
        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for file in photo_files:
                file_path = os.path.join(PHOTO_FOLDER, file)
                zipf.write(file_path, os.path.basename(file))  # ZIP에 추가

        print(f"ZIP 파일 생성 완료: {zip_path}")
    except Exception as e:
        print(f"ZIP 파일 생성 오류: {e}")
        return "ZIP 파일 생성 실패", 500

    return send_file(zip_path, as_attachment=True)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
