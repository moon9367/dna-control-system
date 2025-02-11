#!/bin/bash

# 작업 디렉토리로 이동
cd /home/aiseed/dna-control-system

# 최신 코드 가져오기 (실패 시 계속 진행)
git pull origin main || true

# 파일 이동 (에러 무시)
mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null

# Arduino 코드 업로드
/home/aiseed/dna-control-system/upload_arduino.sh

# 가상환경 활성화
source /home/aiseed/dna-control-system/venv/bin/activate

# Flask 서버 실행
/home/aiseed/dna-control-system/venv/bin/python app.py
