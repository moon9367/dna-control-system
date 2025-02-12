import smtplib
from email.mime.text import MIMEText
import os

# IP 주소 가져오기
def get_ip():
    ip_address = os.popen("hostname -I").read().strip()
    return f"라즈베리파이 IP 주소: {ip_address}"

log_file_path = "/home/aiseed/rc_local_log.txt"

# 로그 작성
try:
    with open(log_file_path, "a") as log_file:
        log_file.write("send_ip_email.py script executed!\n")
except Exception as e:
    print(f"로그 작성 실패: {e}")


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
    send_email()
