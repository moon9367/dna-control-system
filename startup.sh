#!/bin/bash

# 작업 디렉토리로 이동
cd /home/aiseed/dna-control-system

# 가상환경 활성화
source venv/bin/activate

# 최신 코드 가져오기
git pull origin main || true

# 파일 이동
mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null

# Flask 서버 실행
python app.py
