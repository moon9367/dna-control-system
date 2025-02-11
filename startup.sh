#!/bin/bash

LOGFILE=/home/aiseed/startup_detailed.log
exec > $LOGFILE 2>&1  # 로그 기록 시작

echo "Script started at $(date)"

# 디렉토리 이동
cd /home/aiseed/dna-control-system || { echo "Failed to change directory"; exit 1; }
echo "Changed directory to $(pwd)"

# Git Pull 테스트
echo "Starting Git pull..."
git pull origin main || { echo "Git pull failed"; exit 1; }
echo "Git pull completed successfully."

# 파일 이동 테스트
echo "Moving files..."
mv -f index.html templates/ 2>/dev/null || { echo "Failed to move index.html"; true; }
mv -f dna_control.ino dna_control/ 2>/dev/null || { echo "Failed to move dna_control.ino"; true; }
echo "File move completed."

# Arduino 업로드 테스트
echo "Starting Arduino upload..."
/home/aiseed/dna-control-system/upload_arduino.sh || { echo "Arduino upload failed"; true; }
echo "Arduino upload process completed."

# Flask 서버 실행 테스트
echo "Starting Flask server..."
/home/aiseed/dna-control-system/venv/bin/python app.py || { echo "Flask server failed"; true; }
echo "Flask server started."

echo "Script finished at $(date)"
