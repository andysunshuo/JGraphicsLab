"""按已记录的地址和校验值恢复示例资产；日常构建不需要运行此脚本。"""
import hashlib
import json
from pathlib import Path
import urllib.request

root = Path(__file__).resolve().parents[1]
records = json.loads((root / "assets/下载资源清单.json").read_text(encoding="utf-8"))
for record in records:
    target = root / record["file"]
    if target.is_file() and hashlib.sha256(target.read_bytes()).hexdigest() == record["sha256"]:
        print("已经存在：", record["file"])
        continue
    request = urllib.request.Request(record["url"], headers={"User-Agent": "JGraphicsLab educational asset recovery"})
    with urllib.request.urlopen(request, timeout=60) as response:
        data = response.read(16 * 1024 * 1024 + 1)
    if len(data) != record["bytes"] or hashlib.sha256(data).hexdigest() != record["sha256"]:
        raise RuntimeError("资源大小或校验值不符，拒绝覆盖：" + record["file"])
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)
    print("已恢复：", record["file"])
