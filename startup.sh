#!/bin/bash

# 작업 디렉토리로 이동
cd /home/aiseed/dna-control-system

# 최신 코드 가져오기
git pull origin main || true

# 파일 이동
mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null

# Arduino 코드 업로드 (실패해도 계속 진행)
./upload_arduino.sh || true

# Flask 서버 실행
./venv/bin/python app.py
