import serial
from flask import Flask, render_template, request, jsonify

# Flask 설정
app = Flask(__name__)

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

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)