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

# 시리얼 락 설정
serial_lock = threading.Lock()
stop_temp_thread = threading.Event()  # 온도 읽기 일시 중지 플래그

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

# 📡 실시간 온도 저장 변수
current_temperature = "0"

def reset_serial_connection():
    global ser
    try:
        if ser:
            ser.close()
            time.sleep(1)
        ser = find_serial_port()
        print("🔄 시리얼 포트 재연결 시도")
    except Exception as e:
        print(f"❌ 시리얼 포트 재연결 실패: {e}")

def read_temperature():
    global current_temperature
    while True:
        if ser and not stop_temp_thread.is_set():  # 일시 중지 시 스킵
            try:
                with serial_lock:
                    ser.write("g\n".encode())
                    ser.flush()
                    time.sleep(0.2)  # 응답 대기 시간 추가

                    temp = ser.readline().decode().strip()
                    if temp.startswith("Temperature"):
                        current_temperature = temp.split(":")[1].strip()
                        print(f"📡 현재 온도: {current_temperature}°C")
                    else:
                        print(f"⚠️ 예상치 못한 응답: {temp}")
            except Exception as e:
                print(f"❌ 온도 읽기 오류: {e}")
                reset_serial_connection()  # 오류 발생 시 포트 재연결
                current_temperature = "0"
        time.sleep(3)  # 읽기 간격 증가

# 🔥 온도 모니터링 스레드 시작
threading.Thread(target=read_temperature, daemon=True).start()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/temperature")
def get_temperature():
    return jsonify({"temperature": current_temperature})

def send_command_to_arduino(command):
    response = "No response from Arduino"
    if ser:
        with serial_lock:
            stop_temp_thread.set()  # 온도 읽기 일시 중지
            ser.reset_input_buffer()

            print(f"➡️ 아두이노로 명령어 전송: {command.strip()}")  # 명령어 전송 로그

            try:
                ser.write(command.encode())
                ser.flush()
                time.sleep(0.2)  # 명령어 처리 대기 시간 추가

                if ser.in_waiting > 0:
                    response = ser.readline().decode().strip()
                else:
                    print("⚠️ 아두이노 응답 없음 (버퍼 비어 있음)")
            except Exception as e:
                print(f"❌ 명령어 전송 오류: {e}")
                reset_serial_connection()  # 오류 발생 시 포트 재연결

            stop_temp_thread.clear()  # 온도 읽기 재개

    print(f"➡️ 아두이노 응답: {response if response else 'No response'}")
    return response

@app.route("/led", methods=["POST"])
def led_control():
    data = request.get_json()
    action = data["action"].lower()
    command = "a\n" if action == "on" else "b\n"

    print(f"✅ LED 요청 받음: {action}")
    response = send_command_to_arduino(command)
    return jsonify({"message": f"LED {action} 명령 전송 완료", "response": response})

@app.route("/heater", methods=["POST"])
def heater_control():
    data = request.get_json()
    action = data["action"].lower()
    command = "c\n" if action == "on" else "d\n"

    print(f"✅ 히터 요청 받음: {action}")
    response = send_command_to_arduino(command)
    return jsonify({"message": f"Heater {action} 명령 전송 완료", "response": response})

@app.route("/capture", methods=["POST"])
def capture_photo():
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
    if latest_photo_path is None or not os.path.exists(latest_photo_path):
        return jsonify({"error": "사진이 존재하지 않습니다."}), 404

    return jsonify({"photo_name": os.path.basename(latest_photo_path)})

@app.route("/photos/<filename>")
def serve_photo(filename):
    file_path = os.path.join(PHOTO_FOLDER, filename)
    if os.path.exists(file_path):
        return send_file(file_path)
    return "파일을 찾을 수 없습니다.", 404

@app.route("/download_current", methods=["GET"])
def download_current():
    if latest_photo_path is None or not os.path.exists(latest_photo_path):
        return "현재 다운로드할 사진이 없습니다.", 404

    return send_file(latest_photo_path, as_attachment=True)

@app.route("/download_all", methods=["GET"])
def download_all():
    zip_path = os.path.join(PHOTO_FOLDER, "photos.zip")

    photo_files = [f for f in os.listdir(PHOTO_FOLDER) if f.endswith(".jpg")]

    if not photo_files:
        print("❌ 다운로드 실패: 폴더 내 사진 없음")
        return "폴더에 저장된 사진이 없습니다.", 404

    try:
        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for file in photo_files:
                file_path = os.path.join(PHOTO_FOLDER, file)
                zipf.write(file_path, os.path.basename(file))

        print(f"📦 ZIP 파일 생성 완료: {zip_path}")
    except Exception as e:
        print(f"❌ ZIP 파일 생성 오류: {e}")
        return "ZIP 파일 생성 실패", 500

    return send_file(zip_path, as_attachment=True)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
