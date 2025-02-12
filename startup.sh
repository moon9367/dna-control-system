#!/bin/bash

# 최신 코드 가져오기
cd ~/dna-control-system || exit
git pull origin main

# 파일 이동
mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null


~/dna-control-system/upload_arduino.sh

# Flask 서버 실행 (백그라운드에서 실행)
source ~/dna-control-system/venv/bin/activate
python app.py &

echo "✅ Flask 서버 실행 완료!"
