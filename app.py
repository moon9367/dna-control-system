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

# 온도 읽기 스레드 종료 플래그
terminate_temp_thread = threading.Event()

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
        if ser:
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
                reset_serial_connection()  # 포트 재연결 시도

        time.sleep(2)


def send_command(command):
    if ser:
        try:
            ser.reset_input_buffer()  # 버퍼 초기화
            ser.write(f"{command}\n".encode())
            print(f"➡️ 명령어 전송: {command}")
            
            time.sleep(0.3)  # 아두이노의 처리 시간 대기
            response = ser.readline().decode('utf-8', errors='ignore').strip()

            if response:
                print(f"✅ 아두이노 응답: {response}")
                return response
            else:
                print("⚠️ 버퍼에 수신된 데이터 없음")
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
    response = "No response from Arduino"
    retry_count = 3

    if ser:
        with serial_lock:
            stop_temp_thread.set()  # 온도 읽기 일시 중지
            time.sleep(0.5)         # 충돌 방지 대기 시간

            for attempt in range(retry_count):
                print(f"➡️ 명령어 전송 (시도 {attempt + 1}): {command.strip()}")

                try:
                    ser.reset_input_buffer()  # 버퍼 초기화
                    ser.write((command + "\n").encode())
                    ser.flush()
                    print("📡 명령어 전송 완료, 응답 대기 중...")

                    time.sleep(1)  # 아두이노 응답 대기 시간 증가

                    if ser.in_waiting > 0:
                        response = ser.readline().decode().strip()
                        print(f"✅ 아두이노 응답 수신: {response}")
                    else:
                        print("⚠️ 버퍼에 수신된 데이터 없음")

                    if response != "No response from Arduino":
                        break  # 유효한 응답 수신 시 반복 종료

                except Exception as e:
                    print(f"❌ 명령어 전송 오류: {e}")
                    reset_serial_connection()  # 포트 재연결 시도

            stop_temp_thread.clear()  # 온도 읽기 재개

    return response

@app.route("/led", methods=["POST"])
def led_control():
    data = request.get_json()
    action = data["action"].lower()
    command = "LED_ON" if action == "on" else "LED_OFF"

    print(f"✅ LED 요청 받음: {action}")
    response = send_command_to_arduino(command)
    return jsonify({"message": f"LED {action} 명령 전송 완료", "response": response})

@app.route("/heater", methods=["POST"])
def heater_control():
    data = request.get_json()
    action = data["action"].lower()
    command = "HEATER_ON" if action == "on" else "HEATER_OFF"

    print(f"✅ 히터 요청 받음: {action}")
    response = send_command_to_arduino(command)
    return jsonify({"message": f"Heater {action} 명령 전송 완료", "response": response})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)