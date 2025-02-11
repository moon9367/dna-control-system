#!/bin/bash

# 최신 코드 가져오기
cd ~/dna-control-system || exit
git pull origin main

# 파일 이동
mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null


# .ino 파일 경로 확인
INO_PATH=~/dna-control-system/dna_control/dna_control.ino

if [ -f "$INO_PATH" ]; then
    echo "✅ 스케치 파일 찾음: $INO_PATH"
else
    echo "❌ 오류: $INO_PATH 파일을 찾을 수 없습니다!"
    exit 1
fi

# 📡 사용 가능한 시리얼 포트 자동 감지
SERIAL_PORT=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -n 1)

if [ -z "$SERIAL_PORT" ]; then
    echo "⚠️ 오류: 아두이노가 연결된 포트를 찾을 수 없습니다!"
    exit 1
else
    echo "✅ 아두이노 연결된 포트 감지: $SERIAL_PORT"
fi

# 컴파일 (Old Bootloader)
arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old --build-path ~/dna-control-system/build $INO_PATH

# 컴파일 결과 확인
if [ $? -eq 0 ]; then
    echo "✅ 컴파일 성공!"
else
    echo "❌ 컴파일 실패!"
    exit 1
fi

# 업로드 (Old Bootloader)
arduino-cli upload -p $SERIAL_PORT --fqbn arduino:avr:nano:cpu=atmega328old --input-dir ~/dna-control-system/build

# 업로드 결과 확인
if [ $? -eq 0 ]; then
    echo "✅ 업로드 완료!"
else
    echo "❌ 업로드 실패!"
    echo "🔒 시리얼 포트 권한 재설정 시도 중..."
    sudo chmod 666 $SERIAL_PORT

    if [ $? -eq 0 ]; then
        echo "🔓 시리얼 포트 권한 설정 완료: $SERIAL_PORT"
        echo "🔁 업로드 재시도 중..."
        arduino-cli upload -p $SERIAL_PORT --fqbn arduino:avr:nano:cpu=atmega328old --input-dir ~/dna-control-system/build

        if [ $? -eq 0 ]; then
            echo "✅ 업로드 재시도 성공!"
        else
            echo "❌ 업로드 재시도 실패!"
            exit 1
        fi
    else
        echo "❌ 시리얼 포트 권한 설정 실패!"
        exit 1
    fi
fi

# Flask 서버 실행 (백그라운드에서 실행)
source ~/dna-control-system/venv/bin/activate
python app.py &

echo "✅ Flask 서버 실행 완료!"
