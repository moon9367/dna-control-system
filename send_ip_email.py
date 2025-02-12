import smtplib
from email.mime.text import MIMEText
import os
import time

# 네트워크 연결 대기
def wait_for_network():
    for _ in range(10):  # 10초 동안 네트워크 연결 확인
        try:
            os.system("ping -c 1 google.com > /dev/null 2>&1")
            return
        except Exception as e:
            time.sleep(1)  # 1초 대기
    raise Exception("네트워크 연결 실패")

# IP 주소 가져오기
def get_ip():
    ip_address = os.popen("hostname -I").read().strip()
    return f"라즈베리파이 IP 주소: {ip_address}"

# 이메일 전송 함수
def send_email():
    sender_email = "tspol9983@gmail.com"  # 발신 Gmail 주소
    app_password = "fksk leyb lheo nxzo"  # 앱 비밀번호
    recipient_email = "moon93679983@gmail.com"  # 수신 이메일 주소

    # 이메일 내용 작성
    subject = "라즈베리파이 IP 주소"
    message = get_ip()

    msg = MIMEText(message)
    msg["Subject"] = subject
    msg["From"] = sender_email
    msg["To"] = recipient_email

    # Gmail SMTP 서버로 이메일 전송
    try:
        with smtplib.SMTP("smtp.gmail.com", 587) as server:
            server.starttls()
            server.login(sender_email, app_password)
            server.sendmail(sender_email, recipient_email, msg.as_string())
            print("이메일 전송 완료!")
    except Exception as e:
        print(f"이메일 전송 실패: {e}")

if __name__ == "__main__":
    try:
        wait_for_network()
        send_email()
    except Exception as e:
        print(f"스크립트 실행 실패: {e}")
