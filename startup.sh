#!/bin/bash

# 디렉토리 이동
cd /home/aiseed/dna-control-system || { echo "❌ 경로 이동 실패: /home/aiseed/dna-control-system"; exit 1; }

# 최신 코드 가져오기
git config --global --add safe.directory /home/aiseed/dna-control-system
git pull origin main

# 스케치 파일 경로 확인
INO_PATH=/home/aiseed/dna-control-system/dna_control/dna_control.ino
if [ -f "$INO_PATH" ]; then
    echo "✅ 스케치 파일 찾음: $INO_PATH"
else
    echo "❌ 오류: $INO_PATH 파일을 찾을 수 없습니다!"
    exit 1
fi

# 아두이노 업로드 스크립트 실행
/bin/bash /home/aiseed/dna-control-system/upload_arduino.sh

# Flask 서버 실행
source /home/aiseed/dna-control-system/venv/bin/activate
python /home/aiseed/dna-control-system/app.py &
echo "✅ Flask 서버 실행 완료!"
