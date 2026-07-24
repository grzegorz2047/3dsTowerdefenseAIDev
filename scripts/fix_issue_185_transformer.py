#!/usr/bin/env python3
from pathlib import Path

path = Path(__file__).with_name("apply_issue_185.py")
text = path.read_text(encoding="utf-8")
old = '''replace_once(host,
    '\"$BUILD_DIR/seven-segment-digits-tests\"\\n',
    '\"$BUILD_DIR/seven-segment-digits-tests\"\\n\"$BUILD_DIR/mission-pause-tests\"\\n')'''
new = '''replace_once(host,
    '\"$BUILD_DIR/orbit-camera-tests\"\\n\"$BUILD_DIR/seven-segment-digits-tests\"\\n',
    '\"$BUILD_DIR/orbit-camera-tests\"\\n\"$BUILD_DIR/seven-segment-digits-tests\"\\n\"$BUILD_DIR/mission-pause-tests\"\\n')'''
if text.count(old) != 1:
    raise SystemExit(f"expected one transformer anchor, found {text.count(old)}")
path.write_text(text.replace(old, new, 1), encoding="utf-8")
