import asyncio
import shutil
import subprocess
from pathlib import Path

import edge_tts


# Output settings
OUTPUT_DIR = Path.cwd()
VOICE = "zh-CN-YunxiNeural"

FIRST_TEXT = "你好"
SECOND_TEXT = "上海"
SILENCE_SECONDS = 1

FIRST_MP3 = OUTPUT_DIR / "nihao.mp3"
SECOND_MP3 = OUTPUT_DIR / "kaishi.mp3"
SILENCE_WAV = OUTPUT_DIR / "silence_1p5s.wav"
FINAL_MP3 = OUTPUT_DIR / "ld3320_nihao_kaishi_1p5s.mp3"


def find_ffmpeg() -> str:
    # Try PATH first
    ffmpeg_path = shutil.which("ffmpeg")
    if ffmpeg_path:
        return ffmpeg_path

    # Common Windows install locations
    search_dirs = [
        Path.home() / "AppData/Local/Microsoft/WinGet/Packages",
        Path("C:/Program Files/WinGet/Packages"),
        Path("C:/Program Files"),
        Path("C:/Program Files (x86)"),
    ]

    for base_dir in search_dirs:
        if not base_dir.exists():
            continue

        for path in base_dir.rglob("ffmpeg.exe"):
            if "Gyan.FFmpeg" in str(path) or "ShareX" in str(path):
                return str(path)

    raise FileNotFoundError("ffmpeg.exe was not found. Please install FFmpeg first.")


async def generate_tts(text: str, output_file: Path) -> None:
    # Generate TTS audio using Microsoft Edge online TTS
    communicate = edge_tts.Communicate(text=text, voice=VOICE)
    await communicate.save(str(output_file))


def run_command(command: list[str]) -> None:
    print("Running:", " ".join(command))
    subprocess.run(command, check=True)


async def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    ffmpeg = find_ffmpeg()
    print(f"Using FFmpeg: {ffmpeg}")

    print(f"Generating TTS: {FIRST_TEXT}")
    await generate_tts(FIRST_TEXT, FIRST_MP3)

    print(f"Generating TTS: {SECOND_TEXT}")
    await generate_tts(SECOND_TEXT, SECOND_MP3)

    print(f"Generating {SILENCE_SECONDS} seconds silence")
    run_command([
        ffmpeg,
        "-y",
        "-f", "lavfi",
        "-i", "anullsrc=r=44100:cl=mono",
        "-t", str(SILENCE_SECONDS),
        str(SILENCE_WAV),
    ])

    print("Merging final MP3")
    run_command([
        ffmpeg,
        "-y",
        "-i", str(FIRST_MP3),
        "-i", str(SILENCE_WAV),
        "-i", str(SECOND_MP3),
        "-filter_complex",
        "[0:a]aformat=sample_rates=44100:channel_layouts=mono[a0];"
        "[1:a]aformat=sample_rates=44100:channel_layouts=mono[a1];"
        "[2:a]aformat=sample_rates=44100:channel_layouts=mono[a2];"
        "[a0][a1][a2]concat=n=3:v=0:a=1[out]",
        "-map", "[out]",
        "-codec:a", "libmp3lame",
        "-b:a", "192k",
        str(FINAL_MP3),
    ])

    print()
    print("Done!")
    print(f"Final file: {FINAL_MP3}")


if __name__ == "__main__":
    asyncio.run(main())