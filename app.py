import os
import serial
import time
from datetime import datetime
from flask import Flask, render_template, request, send_file, jsonify
from picamera2 import Picamera2
import zipfile

# Flask 설정
app = Flask(__name__)
picam2 = Picamera2()

# 해상도 설정 (예: 1920x1080)
config = picam2.create_still_configuration(main={"size": (1920, 1080)})
picam2.configure(config)
picam2.start()

# 사진 저장 경로
PHOTO_FOLDER = "/home/aiseed/photos"
if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

latest_photo_path = None  # 최신 사진 경로 저장 변수

# 시리얼 포트 설정 (Arduino 연결)
try:
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    time.sleep(2)  # 시리얼 초기화 대기
except Exception as e:
    print(f"시리얼 포트 연결 실패: {e}")
    ser = None


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/set_temp", methods=["POST"])
def set_temp():
    target_temp = request.form["temperature"]
    if ser:
        ser.write(f"SET_TEMP:{target_temp}\n".encode())
        return f"온도 설정: {target_temp}°C"
    return "시리얼 포트 오류!", 500


@app.route("/heater", methods=["POST"])
def heater_control():
    action = request.form["action"]
    if ser:
        ser.write(f"HEATER_{action}\n".encode())
        return f"Heater {action}"
    return "시리얼 포트 오류!", 500


@app.route("/led", methods=["POST"])
def led_control():
    action = request.form["action"]
    if ser:
        ser.write(f"LED_{action}\n".encode())
        return f"LED {action}"
    return "시리얼 포트 오류!", 500


@app.route("/temperature")
def get_temperature():
    if ser:
        ser.write("GET_TEMP\n".encode())  # 온도 요청
        time.sleep(0.1)
        temperature = ser.readline().decode().strip()
        led_status = ser.readline().decode().strip()
        heater_status = ser.readline().decode().strip()

        return jsonify({
            "temperature": temperature or "N/A",
            "led": "ON" if led_status == "LED_ON" else "OFF",
            "heater": "ON" if heater_status == "HEATER_ON" else "OFF",
        })
    return jsonify({"error": "시리얼 포트 오류!"})


@app.route("/capture", methods=["POST"])
def capture_photo():
    global latest_photo_path
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest_photo_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")
    picam2.capture_file(latest_photo_path)
    return f"사진 촬영 완료: {latest_photo_path}"


@app.route("/download_current", methods=["GET"])
def download_current():
    if latest_photo_path and os.path.exists(latest_photo_path):
        return send_file(latest_photo_path, as_attachment=True)
    return "현재 다운로드할 사진이 없습니다.", 404


@app.route("/download_all", methods=["GET"])
def download_all():
    zip_path = "/home/aiseed/photos/photos.zip"
    with zipfile.ZipFile(zip_path, 'w') as zipf:
        for root, _, files in os.walk(PHOTO_FOLDER):
            for file in files:
                zipf.write(os.path.join(root, file), file)
    return send_file(zip_path, as_attachment=True)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
