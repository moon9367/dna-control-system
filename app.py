import os
import serial
import threading
import time
from datetime import datetime
from flask import Flask, render_template, request, jsonify, send_file
from picamera2 import Picamera2, Preview
import zipfile

# Flask 설정
app = Flask(__name__)
picam2 = Picamera2()

config = picam2.create_still_configuration(main={"size": (1920, 1080)})
picam2.configure(config)
picam2.rotation = 180  # ++ 상하 반전 유지
picam2.start()

# ++ 카메라 자동 초점 활성화
picam2.set_controls({"AfMode": 2})  # 2는 연속 자동 초점 모드

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

# 온도 읽기 스레드 종료 플래그
terminate_temp_thread = threading.Event()

def read_temperature():
    global current_temperature
    while not terminate_temp_thread.is_set():
        if ser and not stop_temp_thread.is_set():
            try:
                ser.reset_input_buffer()  # 버퍼 초기화
                ser.write("GET_TEMP\n".encode())
                time.sleep(0.5)

                raw_data = ser.readline()
                try:
                    temp_data = raw_data.decode('utf-8').strip()
                    if temp_data.startswith("Temperature:"):
                        current_temperature = temp_data.split(":")[1].strip()
                        print(f"📡 현재 온도: {current_temperature}°C")
                except UnicodeDecodeError:
                    print(f"⚠️ 잘못된 데이터 수신: {raw_data}")  # 디코딩 실패 시 원본 데이터 표시

            except Exception as e:
                print(f"❌ 온도 읽기 오류: {e}")

        time.sleep(8)

def send_command(command):
    if ser:
        try:
            ser.reset_input_buffer()  # 버퍼 초기화
            ser.write(f"{command}\n".encode())
            print(f"➡️ 명령어 전송: {command}")
            
            time.sleep(0.3)  # 아두이노의 처리 시간 대기
            
            # 응답 읽기 및 필터링
            response = None
            for _ in range(5):  # 최대 5번 재시도
                raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
                if raw_data and not raw_data.startswith("Temperature"):
                    response = raw_data
                    break
                time.sleep(0.1)  # 짧은 대기 후 재시도

            if response:
                print(f"✅ 아두이노 응답: {response}")
                return response
            else:
                print("⚠️ 버퍼에 유효한 데이터 없음")
                return None

        except Exception as e:
            print(f"❌ 명령어 전송 오류: {e}")
            return None

# 🔥 온도 모니터링 스레드 시작
temp_thread = threading.Thread(target=read_temperature, daemon=True)
temp_thread.start()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/temperature")
def get_temperature():
    return jsonify({"temperature": current_temperature})

def send_command_to_arduino(command):
    if ser:
        with serial_lock:
            print("⏸️ 온도 출력 대기")
            stop_temp_thread.set()  # 온도 읽기 일시 중지
            time.sleep(0.5)         # 딜레이 추가

            try:
                ser.reset_input_buffer()  # 버퍼 초기화
                ser.write((command + "\n").encode())
                ser.flush()
                print(f"➡️ 명령어 전송: {command.strip()}")

                time.sleep(1.5)  # ++ 아두이노 응답 대기 시간 연장

                # 응답 필터링 (온도 데이터 제외)
                response = None
                for _ in range(5):
                    raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
                    if raw_data and not raw_data.startswith("Temperature"):
                        response = raw_data
                        break
                    time.sleep(0.1)

                if response:
                    print(f"✅ 아두이노 응답 수신: {response}")
                else:
                    response = "No response from Arduino"
                    print("⚠️ 버퍼에 유효한 데이터 없음")

            except Exception as e:
                print(f"❌ 명령어 전송 오류: {e}")
                response = "No response from Arduino"

            time.sleep(0.5)  # 딜레이 추가
            stop_temp_thread.clear()  # 온도 읽기 재개
            print("▶️ 온도 출력 시작")

            return response

@app.route("/led/on", methods=["POST"])
def led_on():
    command = "LED_ON"
    print("✅ LED 켜기 요청 수신")
    response = send_command_to_arduino(command)
    return jsonify({"message": "LED 켜기 완료", "response": response})

@app.route("/led/off", methods=["POST"])
def led_off():
    command = "LED_OFF"
    print("✅ LED 끄기 요청 수신")
    response = send_command_to_arduino(command)
    return jsonify({"message": "LED 끄기 완료", "response": response})

@app.route("/heater/on", methods=["POST"])
def heater_on():
    command = "HEATER_ON"
    print("✅ 히터 켜기 요청 수신")
    response = send_command_to_arduino(command)
    return jsonify({"message": "히터 켜기 완료", "response": response})

@app.route("/heater/off", methods=["POST"])
def heater_off():
    command = "HEATER_OFF"
    print("✅ 히터 끄기 요청 수신")
    response = send_command_to_arduino(command)
    return jsonify({"message": "히터 끄기 완료", "response": response})

@app.route("/capture", methods=["POST"])
def capture_photo():
    global latest_photo_path

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest_photo_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")

    try:
        picam2.set_controls({"AfTrigger": 1})  # ++ 사진 촬영 전 초점 재조정
        time.sleep(1)  # ++ 초점 조정 대기 시간
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
