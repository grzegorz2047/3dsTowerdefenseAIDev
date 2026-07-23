from pathlib import Path

path = Path('source/UiRenderer.cpp')
text = path.read_text()
text = text.replace('"KAMPANIA", true);', '"X KAMPANIA", true);')
text = text.replace('"POWTORZ", false);', '"Y POWTORZ", false);')
if '"X KAMPANIA"' not in text or '"Y POWTORZ"' not in text:
    raise SystemExit('result labels were not updated')
path.write_text(text)
Path('.github/scripts/issue157_compile_fix.py').unlink()
