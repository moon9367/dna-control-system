import os
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
PHOTO_FOLDER = "/home/aiseed/photos"  # 사용자 계정에 맞게 변경
if not os.path.exists(PHOTO_FOLDER):
    os.makedirs(PHOTO_FOLDER)

latest_photo_path = None  # 최신 사진 경로를 저장하는 변수


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/set_temp", methods=["POST"])
def set_temp():
    target_temp = request.form["temperature"]
    # 시리얼 통신 코드 추가 필요
    return f"온도 설정: {target_temp}°C"


@app.route("/heater", methods=["POST"])
def heater_control():
    action = request.form["action"]
    # 시리얼 통신 코드 추가 필요
    return f"Heater {action}"


@app.route("/led", methods=["POST"])
def led_control():
    action = request.form["action"]
    # 시리얼 통신 코드 추가 필요
    return f"LED {action}"


@app.route("/temperature")
def get_temperature():
    # 시리얼 통신에서 데이터 읽기 구현 필요
    temperature = "25.0"  # 예제 데이터
    led_status = "OFF"    # 예제 데이터
    heater_status = "OFF" # 예제 데이터
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
    camera.capture(latest_photo_path)
    return f"사진 촬영 완료: {latest_photo_path}"


@app.route("/download_current", methods=["GET"])
def download_current():
    global latest_photo_path
    if latest_photo_path and os.path.exists(latest_photo_path):
        return send_file(latest_photo_path, as_attachment=True)
    else:
        return "현재 다운로드할 사진이 없습니다.", 404


@app.route("/download_all", methods=["GET"])
def download_all():
    zip_path = "/home/pi/photos/photos.zip"
    with zipfile.ZipFile(zip_path, 'w') as zipf:
        for root, dirs, files in os.walk(PHOTO_FOLDER):
            for file in files:
                zipf.write(os.path.join(root, file), file)
    return send_file(zip_path, as_attachment=True)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
