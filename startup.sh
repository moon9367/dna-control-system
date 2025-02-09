#!/bin/bash

# 작업 디렉토리로 이동
cd ~/dna-control-system

# 최신 코드 가져오기
git pull origin main

# 파일 이동
mv -f index.html templates/
mv -f dna_control.ino dna_control/

# Arduino 코드 업로드
~/dna-control-system/upload_arduino.sh

# 가상환경 활성화
source venv/bin/activate

# Flask 서버 실행
python app.py
