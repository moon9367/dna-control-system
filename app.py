import os
import serial
import threading
import time
from datetime import datetime
from flask import Flask, render_template, request, jsonify, send_file
from picamera2 import Picamera2
import zipfile

# Flask 설정
app = Flask(__name__)
picam2 = Picamera2()

# 카메라 설정
config = picam2.create_still_configuration(main={"size": (1920, 1080)})
picam2.configure(config)
picam2.start()

# 사진 저장 폴더
PHOTO_FOLDER = "/home/aiseed/photos"
latest_photo_path = None  # 최신 사진 경로 저장 변수

# 폴더가 없으면 생성
if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

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

# LED 및 PTC 히터 명령 코드
led_on = "a"
led_off = "b"
heater_on = "c"
heater_off = "d"

# 📡 실시간 온도 저장 변수
current_temperature = "0"

def read_temperature():
    """아두이노에서 지속적으로 온도 데이터를 읽어와 저장하는 백그라운드 작업"""
    global current_temperature
    while True:
        if ser:
            try:
                ser.write("g\n".encode())  # 온도 요청
                temp = ser.readline().decode().strip()
                if temp:
                    current_temperature = temp
                    print(f"📡 현재 온도: {current_temperature}°C")
            except Exception as e:
                print(f"❌ 온도 읽기 오류: {e}")
                current_temperature = "0"
        time.sleep(2)  # 2초마다 온도 요청

# 🔥 온도 모니터링 스레드 시작
threading.Thread(target=read_temperature, daemon=True).start()

@app.route("/")
def index():
    """웹 UI 렌더링"""
    return render_template("index.html")

@app.route("/temperature")
def get_temperature():
    """현재 저장된 온도 데이터를 반환"""
    return jsonify({"temperature": current_temperature})

@app.route("/led", methods=["POST"])
def led_control():
    """LED ON/OFF 제어"""
    data = request.get_json()
    action = data["action"].lower()
    command = "a" if action == "on" else "b"

    print(f"✅ LED 요청 받음: {action}")  # 요청 확인
    print(f"➡️ 아두이노로 전송: {command}")  # 아두이노로 보낼 값 확인

    if ser:
        ser.write(f"{command}\n".encode())  # 아두이노로 명령 전송
        ser.flush()  # 시리얼 버퍼 초기화
    else:
        print("⚠️ 시리얼 포트 연결 안됨!")

    return jsonify({"message": f"LED {action} 명령 전송 완료"})


@app.route("/heater", methods=["POST"])
def heater_control():
    """PTC 히터 ON/OFF 제어"""
    data = request.get_json()
    action = data["action"].lower()
    command = heater_on if action == "on" else heater_off
    ser.write(f"{command}\n".encode())
    return jsonify({"message": f"Heater {action} 명령 전송 완료"})

@app.route("/capture", methods=["POST"])
def capture_photo():
    """사진 촬영 후 최신 사진 파일명을 저장하고 반환"""
    global latest_photo_path

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest_photo_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")

    try:
        picam2.capture_file(latest_photo_path)
        print(f"📸 사진 촬영 완료: {latest_photo_path}")

    except Exception as e:
        print(f"❌ 사진 촬영 오류: {e}")
        return jsonify({"error": "사진 촬영 실패"}), 500

    return jsonify({"message": "사진 촬영 완료", "photo_name": os.path.basename(latest_photo_path)})

@app.route("/latest_photo", methods=["GET"])
def get_latest_photo():
    """현재 최신 사진 파일명을 웹으로 전달"""
    if latest_photo_path is None or not os.path.exists(latest_photo_path):
        return jsonify({"error": "사진이 존재하지 않습니다."}), 404

    return jsonify({"photo_name": os.path.basename(latest_photo_path)})

@app.route("/photos/<filename>")
def serve_photo(filename):
    """웹에서 특정 파일 요청 시 제공"""
    file_path = os.path.join(PHOTO_FOLDER, filename)
    if os.path.exists(file_path):
        return send_file(file_path)
    return "파일을 찾을 수 없습니다.", 404

@app.route("/download_current", methods=["GET"])
def download_current():
    """현재 최신 사진 다운로드"""
    if latest_photo_path is None or not os.path.exists(latest_photo_path):
        return "현재 다운로드할 사진이 없습니다.", 404

    return send_file(latest_photo_path, as_attachment=True)

@app.route("/download_all", methods=["GET"])
def download_all():
    """저장된 모든 사진을 ZIP 파일로 다운로드"""
    zip_path = os.path.join(PHOTO_FOLDER, "photos.zip")

    # 폴더 내 파일 확인
    photo_files = [f for f in os.listdir(PHOTO_FOLDER) if f.endswith(".jpg")]

    if not photo_files:
        print("❌ 다운로드 실패: 폴더 내 사진 없음")
        return "폴더에 저장된 사진이 없습니다.", 404

    try:
        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for file in photo_files:
                file_path = os.path.join(PHOTO_FOLDER, file)
                zipf.write(file_path, os.path.basename(file))  # ZIP에 추가

        print(f"📦 ZIP 파일 생성 완료: {zip_path}")
    except Exception as e:
        print(f"❌ ZIP 파일 생성 오류: {e}")
        return "ZIP 파일 생성 실패", 500

    return send_file(zip_path, as_attachment=True)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
