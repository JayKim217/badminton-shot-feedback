import argparse
import subprocess
from pathlib import Path
import tempfile


def run(cmd):
    print(" ".join(cmd))
    subprocess.run(cmd, check=True)


def download_clip(url: str, start: str, end: str, output: str):
    output_path = Path(output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as tmpdir:
        tmp_file = Path(tmpdir) / "raw_clip.mp4"

        section = f"*{start}-{end}"

        # 1. Download
        download_cmd = [
            "yt-dlp",
            "--download-sections",
            section,
            "-f",
            "bv*+ba/b",
            "--merge-output-format",
            "mp4",
            "-o",
            str(tmp_file),
            url,
        ]

        print("\nDownloading clip...")
        run(download_cmd)

        # 2. Convert → 480p + QuickTime compatible
        convert_cmd = [
            "ffmpeg",
            "-y",
            "-i",
            str(tmp_file),

            "-vf",
            "scale=854:-2,fps=30",

            "-c:v",
            "libx264",
            "-pix_fmt",
            "yuv420p",
            "-preset",
            "medium",
            "-crf",
            "23",

            "-c:a",
            "aac",
            "-b:a",
            "128k",

            "-movflags",
            "+faststart",

            str(output_path),
        ]

        print("\nConverting to 480p QuickTime-compatible MP4...")
        run(convert_cmd)

    print("\nFinished.")
    print(f"Saved to: {output_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Download and clip a YouTube video."
    )

    parser.add_argument("url")
    parser.add_argument("start")
    parser.add_argument("end")

    parser.add_argument(
        "-o",
        "--output",
        default="input/clip.mp4",
        help="Output path",
    )

    args = parser.parse_args()

    download_clip(
        url=args.url,
        start=args.start,
        end=args.end,
        output=args.output,
    )


if __name__ == "__main__":
    main()