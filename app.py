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

# 아두이노 시리얼 포트 설정
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)

# 사진 저장 경로
PHOTO_FOLDER = "/home/aiseed/photos"
STATIC_PHOTO_PATH = "/home/aiseed/dna-control-system/static/photo.jpg"

if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

latest_photo_path = STATIC_PHOTO_PATH  # 최신 사진 기본 경로


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/set_temp", methods=["POST"])
def set_temp():
    target_temp = request.form["temperature"]
    ser.write(f"SET_TEMP:{target_temp}\n".encode())  # 아두이노에 온도 설정 전송
    return jsonify({"message": f"온도 설정: {target_temp}°C"})


@app.route("/heater", methods=["POST"])
def heater_control():
    action = request.form["action"]
    ser.write(f"HEATER_{action}\n".encode())  # 아두이노에 히터 제어 명령 전송
    return jsonify({"message": f"Heater {action}"})


@app.route("/led", methods=["POST"])
def led_control():
    action = request.form["action"]
    ser.write(f"LED_{action}\n".encode())  # 아두이노에 LED 제어 명령 전송
    return jsonify({"message": f"LED {action}"})


@app.route("/temperature")
def get_temperature():
    ser.write("GET_TEMP\n".encode())  # 온도 데이터 요청
    temperature = ser.readline().decode().strip()
    led_status = "ON" if ser.readline().decode().strip() == "LED_ON" else "OFF"
    heater_status = "ON" if ser.readline().decode().strip() == "HEATER_ON" else "OFF"

    return jsonify({
        "temperature": temperature,
        "led": led_status,
        "heater": heater_status,
    })


@app.route("/capture", methods=["POST"])
def capture_photo():
    global latest_photo_path
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest_photo_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")
    
    picam2.capture_file(latest_photo_path)  # 최신 사진 촬영
    os.system(f"cp {latest_photo_path} {STATIC_PHOTO_PATH}")  # 웹에서 최신 사진 갱신

    return jsonify({"message": "사진 촬영 완료", "photo_url": "/static/photo.jpg"})


@app.route("/download_current", methods=["GET"])
def download_current():
    if os.path.exists(latest_photo_path):
        return send_file(latest_photo_path, as_attachment=True)
    else:
        return "현재 다운로드할 사진이 없습니다.", 404


@app.route("/download_all", methods=["GET"])
def download_all():
    zip_path = "/home/aiseed/photos/photos.zip"
    with zipfile.ZipFile(zip_path, 'w') as zipf:
        for root, dirs, files in os.walk(PHOTO_FOLDER):
            for file in files:
                zipf.write(os.path.join(root, file), file)
    return send_file(zip_path, as_attachment=True)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
