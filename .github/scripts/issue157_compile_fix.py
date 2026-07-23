from pathlib import Path

path = Path('source/Renderer.cpp')
text = path.read_text()
text = text.replace('void appendBallistaTower(std::vector<Vertex>& v)', 'void appendTower(std::vector<Vertex>& v)')
text = text.replace('appendBallistaTower(vertices)', 'appendTower(vertices)')
if 'void appendTower' not in text:
    raise SystemExit('appendTower compatibility name missing')
path.write_text(text)
Path('.github/scripts/issue157_compile_fix.py').unlink()
