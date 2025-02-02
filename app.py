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

if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

# 시리얼 포트 자동 감지
def find_serial_port():
    import glob
    possible_ports = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
    for port in possible_ports:
        try:
            return serial.Serial(port, 9600, timeout=1)
        except:
            continue
    return None

ser = find_serial_port()
current_temperature = "0"

# 온도 읽기 스레드
def read_temperature():
    global current_temperature
    while True:
        if ser:
            try:
                ser.write("GET_TEMP\n".encode())
                temp = ser.readline().decode().strip()
                if temp.startswith("Temperature:"):
                    current_temperature = temp.split(":")[1].strip()
                    print(f"📡 현재 온도: {current_temperature}°C")
            except Exception as e:
                print(f"❌ 온도 읽기 오류: {e}")
        time.sleep(2)

threading.Thread(target=read_temperature, daemon=True).start()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/temperature")
def get_temperature():
    return jsonify({"temperature": current_temperature})

@app.route("/led", methods=["POST"])
def led_control():
    action = request.form["action"].upper()
    command = f"LED_{action}\n"
    ser.write(command.encode())
    time.sleep(0.5)
    response = ser.readline().decode().strip()
    return jsonify({"message": response})

@app.route("/heater", methods=["POST"])
def heater_control():
    action = request.form["action"].upper()
    command = f"HEATER_{action}\n"
    ser.write(command.encode())
    time.sleep(0.5)
    response = ser.readline().decode().strip()
    return jsonify({"message": response})

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

@app.route("/download_current", methods=["GET"])
def download_current():
    if latest_photo_path and os.path.exists(latest_photo_path):
        return send_file(latest_photo_path, as_attachment=True)
    return "현재 다운로드할 사진이 없습니다.", 404

@app.route("/download_all", methods=["GET"])
def download_all():
    zip_path = os.path.join(PHOTO_FOLDER, "photos.zip")
    photo_files = [f for f in os.listdir(PHOTO_FOLDER) if f.endswith(".jpg")]

    if not photo_files:
        return "폴더에 저장된 사진이 없습니다.", 404

    try:
        with zipfile.ZipFile(zip_path, 'w') as zipf:
            for file in photo_files:
                file_path = os.path.join(PHOTO_FOLDER, file)
                zipf.write(file_path, os.path.basename(file))
    except Exception as e:
        return "ZIP 파일 생성 실패", 500

    return send_file(zip_path, as_attachment=True)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
