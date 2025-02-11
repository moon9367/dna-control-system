#!/bin/bash

# 로그 파일 설정
LOGFILE=/home/aiseed/startup.log
exec > $LOGFILE 2>&1

echo "Script started at $(date)"

# 작업 디렉토리로 이동
cd /home/aiseed/dna-control-system || { echo "Failed to change directory"; exit 1; }
echo "Changed directory successfully."

# 최신 코드 가져오기
git pull origin main || { echo "Git pull failed"; true; }
echo "Git pull completed."

# 파일 이동
mv -f index.html templates/ 2>/dev/null || { echo "Failed to move index.html"; true; }
mv -f dna_control.ino dna_control/ 2>/dev/null || { echo "Failed to move dna_control.ino"; true; }
echo "File move completed."

# Arduino 코드 업로드
/home/aiseed/dna-control-system/upload_arduino.sh || { echo "Arduino upload failed"; true; }
echo "Arduino upload process completed."

# Flask 서버 실행
./venv/bin/python app.py || { echo "Flask server failed to start"; true; }
echo "Flask server started successfully."
