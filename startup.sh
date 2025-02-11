#!/bin/bash

# 환경 변수 및 실행 경로 확인
echo "Current working directory: $(pwd)" > /home/aiseed/systemd_debug.log
echo "Python path: $(which python)" >> /home/aiseed/systemd_debug.log
echo "Environment variables:" >> /home/aiseed/systemd_debug.log
env >> /home/aiseed/systemd_debug.log

# 작업 디렉토리로 이동
cd /home/aiseed/dna-control-system || exit

# 가상환경 활성화
source venv/bin/activate

# PYTHONPATH 설정
export PYTHONPATH=/home/aiseed/dna-control-system/venv/lib/python3.9/site-packages

# 최신 코드 가져오기
git pull origin main || true

# 파일 이동
mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null

# Arduino 코드 업로드 (실패 무시)
./upload_arduino.sh || true

# Flask 서버 실행
python app.py
