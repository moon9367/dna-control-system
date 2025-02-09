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

# 카메라 설정
config = picam2.create_still_configuration(main={"size": (1920, 1080)})
picam2.configure(config)
picam2.rotation = 180  # 상하 반전
picam2.start()

# 카메라 초점 모드 설정
picam2.set_controls({"AfMode": 2})  # 연속 자동 초점 모드


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
def read_temperature():
    global current_temperature
    while not stop_temp_thread.is_set():
        if ser:
            try:
                with serial_lock:
                    ser.reset_input_buffer()
                    raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
                    print(f"Raw Data: {raw_data}")  # 디버깅용 로그 출력

                    if raw_data.startswith("Temperature:"):
                        current_temperature = float(raw_data.split(":")[1].strip())
                        print(f"📡 현재 온도: {current_temperature}°C")
            except Exception as e:
                print(f"온도 읽기 오류: {e}")
        time.sleep(2)




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
            stop_temp_thread.set()  # 온도 읽기 일시 중지
            time.sleep(0.5)         # 명령 전 대기

            try:
                ser.reset_input_buffer()
                ser.write((command + "\n").encode())
                print(f"➡️ 명령어 전송: {command}")

                time.sleep(1.5)  # 아두이노 응답 대기

                response = None
                for _ in range(10):  # 최대 10회 재시도
                    raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
                    print(f"🔄 수신 데이터: {raw_data}")  # 디버깅용 로그

                    # "Temperature:"로 시작하는 데이터는 온도로 처리
                    if raw_data.startswith("Temperature:"):
                        global current_temperature
                        current_temperature = float(raw_data.split(":")[1].strip())
                        print(f"📡 현재 온도: {current_temperature}°C")
                    elif raw_data.startswith("CMD:"):
                        response = raw_data.split(":")[1].strip()
                        break

                if response:
                    print(f"✅ 명령 응답 수신: {response}")
                else:
                    print("⚠️ 유효한 명령 응답 없음")

            except Exception as e:
                print(f"❌ 명령어 전송 오류: {e}")

            finally:
                stop_temp_thread.clear()  # 온도 읽기 재개
                print("▶️ 온도 읽기 재개")

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

@app.route('/capture', methods=['POST'])
def capture_photo():
    global latest_photo_path
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    latest_photo_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")

    try:
        picam2.set_controls({"AfTrigger": 1})  # 초점 조정
        time.sleep(1)  # 초점 조정 대기
        picam2.capture_file(latest_photo_path)
        print(f"📸 사진 촬영 완료: {latest_photo_path}")
        return jsonify({"message": "사진 촬영 완료", "photo_name": os.path.basename(latest_photo_path)})
    except Exception as e:
        print(f"사진 촬영 오류: {e}")
        return jsonify({"error": "사진 촬영 실패"}), 500

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

@app.route('/delete_all_photos', methods=['POST'])
def delete_all_photos():
    for file in os.listdir(PHOTO_FOLDER):
        file_path = os.path.join(PHOTO_FOLDER, file)
        if os.path.isfile(file_path):
            os.remove(file_path)
    return jsonify({"message": "모든 사진이 삭제되었습니다."})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
