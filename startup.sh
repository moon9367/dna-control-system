#!/bin/bash
LOGFILE=/home/aiseed/startup_detailed.log
exec > $LOGFILE 2>&1

echo "Script started at $(date)"
cd /home/aiseed/dna-control-system || { echo "Directory change failed"; exit 1; }
echo "Changed directory successfully"

git pull origin main || { echo "Git pull failed"; exit 1; }
echo "Git pull completed"

mv -f index.html templates/ 2>/dev/null
mv -f dna_control.ino dna_control/ 2>/dev/null
echo "File move completed"

./upload_arduino.sh || { echo "Arduino upload failed"; exit 1; }
echo "Arduino upload completed"

./venv/bin/python app.py || { echo "Flask server failed"; exit 1; }
echo "Flask server started"
