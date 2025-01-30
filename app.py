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
    print("⚠️  Arduino 연결 실패: USB 포트 확인 필요")
    return None

ser = find_serial_port()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/set_temp", methods=["POST"])
def set_temp():
    """목표 온도 설정"""
    if not ser:
        return jsonify({"error": "시리얼 포트 연결 실패"}), 500
    
    data = request.get_json()
    target_temp = data["temperature"]
    ser.write(f"SET_TEMP:{target_temp}\n".encode())

    response = ser.readline().decode().strip()
    return jsonify({"message": f"온도 설정: {target_temp}°C", "response": response})

@app.route("/heater", methods=["POST"])
def heater_control():
    """히터 ON/OFF 제어"""
    if not ser:
        return jsonify({"error": "시리얼 포트 연결 실패"}), 500

    data = request.get_json()
    action = data["action"].upper()
    ser.write(f"HEATER_{action}\n".encode())

    response = ser.readline().decode().strip()
    return jsonify({"message": f"Heater {action}", "response": response})

@app.route("/led", methods=["POST"])
def led_control():
    """LED ON/OFF 제어"""
    if not ser:
        return jsonify({"error": "시리얼 포트 연결 실패"}), 500

    data = request.get_json()
    action = data["action"].upper()
    ser.write(f"LED_{action}\n".encode())

    response = ser.readline().decode().strip()
    return jsonify({"message": f"LED {action}", "response": response})

@app.route("/temperature")
def get_temperature():
    """현재 온도, LED, 히터 상태 가져오기"""
    if not ser:
        return jsonify({"error": "시리얼 포트 연결 실패"}), 500

    ser.write("GET_TEMP\n".encode())
    
    response = ser.readlines()
    temp, led, heater = "", "", ""

    for line in response:
        line = line.decode().strip()
        if line.startswith("TEMP:"):
            temp = line.split(":")[1]
        elif line.startswith("LED:"):
            led = line.split(":")[1]
        elif line.startswith("HEATER:"):
            heater = line.split(":")[1]

    return jsonify({
        "temperature": temp,
        "led": led,
        "heater": heater,
    })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
