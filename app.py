import os
import serial
from flask import Flask, render_template, request, send_file, jsonify
from picamera2 import Picamera2
from datetime import datetime

# Flask 설정
app = Flask(__name__)

# 카메라 설정
picam2 = Picamera2()
config = picam2.create_still_configuration(main={"size": (1920, 1080)})
picam2.configure(config)
picam2.start()

# 사진 저장 폴더
PHOTO_FOLDER = "/home/pi/photos"
if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

# 시리얼 포트 설정
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/set_temp", methods=["POST"])
def set_temp():
    target_temp = request.form["temperature"]
    ser.write(f"SET_TEMP:{target_temp}\n".encode())
    return f"온도 설정: {target_temp}°C"

@app.route("/heater", methods=["POST"])
def heater_control():
    action = request.form["action"]
    ser.write(f"HEATER_{action}\n".encode())
    return f"Heater {action}"

@app.route("/led", methods=["POST"])
def led_control():
    action = request.form["action"]
    ser.write(f"LED_{action}\n".encode())
    return f"LED {action}"

@app.route("/temperature")
def get_temperature():
    ser.write("GET_TEMP\n".encode())
    temperature = ser.readline().decode().strip()
    led_status = "ON" if ser.readline().decode().strip() == "LED_ON" else "OFF"
    heater_status = "ON" if ser.readline().decode().strip() == "HEATER_ON" else "OFF"
    return {
        "temperature": temperature,
        "led": led_status,
        "heater": heater_status,
    }

@app.route("/capture", methods=["POST"])
def capture_photo():
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    file_path = os.path.join(PHOTO_FOLDER, f"photo_{timestamp}.jpg")
    try:
        picam2.capture_file(file_path)
        print(f"📸 사진 촬영 완료: {file_path}")
        return send_file(file_path, mimetype="image/jpeg")
    except Exception as e:
        print(f"❌ 사진 촬영 오류: {e}")
        return jsonify({"error": "사진 촬영 실패"}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)