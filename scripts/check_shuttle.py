import argparse
from pathlib import Path

import cv2
import pandas as pd


def load_court_points(path):
    points = []

    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            x, y = line.split(";")
            points.append((int(round(float(x))), int(round(float(y)))))

    if len(points) != 6:
        raise ValueError(f"Expected 6 court points, got {len(points)}")

    return points


def draw_court(frame, court_points):
    p1, p2, p3, p4, p5, p6 = court_points

    boundary_color = (255, 0, 0)
    net_color = (255, 255, 0)
    point_color = (0, 0, 255)
    thickness = 3

    cv2.line(frame, p1, p4, boundary_color, thickness)
    cv2.line(frame, p4, p3, boundary_color, thickness)
    cv2.line(frame, p3, p2, boundary_color, thickness)
    cv2.line(frame, p2, p1, boundary_color, thickness)

    cv2.line(frame, p5, p6, net_color, thickness)

    for idx, pt in enumerate(court_points, start=1):
        cv2.circle(frame, pt, 7, point_color, -1)
        cv2.putText(
            frame,
            str(idx),
            (pt[0] + 8, pt[1] - 8),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            point_color,
            2,
        )


def load_shuttle_predictions(csv_path):
    df = pd.read_csv(csv_path)
    df.columns = [c.strip() for c in df.columns]

    required_cols = {"Frame", "Visibility", "X", "Y"}
    missing = required_cols - set(df.columns)

    if missing:
        raise ValueError(f"Missing columns in shuttle CSV: {missing}")

    preds = {
        int(row["Frame"]): (
            int(row["Visibility"]),
            int(row["X"]),
            int(row["Y"]),
        )
        for _, row in df.iterrows()
    }

    return preds


def main():
    parser = argparse.ArgumentParser(
        description="Visualize shuttle tracking and/or court detection on a video."
    )

    parser.add_argument(
        "input_video",
        help="Path to input video",
    )

    parser.add_argument(
        "results_csv",
        nargs="?",
        default=None,
        help="Optional TrackNetV3 result CSV",
    )

    parser.add_argument(
        "--court_points",
        default=None,
        help="Optional path to court_points.txt",
    )

    parser.add_argument(
        "--output",
        default=None,
        help="Output video path",
    )

    parser.add_argument(
        "--trail",
        type=int,
        default=12,
        help="Number of previous shuttle points to draw",
    )

    args = parser.parse_args()

    video_path = Path(args.input_video)

    if args.output is None:
        output_path = video_path.with_name(video_path.stem + "_checked.mp4")
    else:
        output_path = Path(args.output)

    output_path.parent.mkdir(parents=True, exist_ok=True)

    court_points = None
    if args.court_points is not None:
        court_points = load_court_points(args.court_points)

    preds = {}
    if args.results_csv is not None:
        preds = load_shuttle_predictions(args.results_csv)

    cap = cv2.VideoCapture(str(video_path))

    if not cap.isOpened():
        raise RuntimeError(f"Could not open video: {video_path}")

    fps = cap.get(cv2.CAP_PROP_FPS)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    writer = cv2.VideoWriter(
        str(output_path),
        cv2.VideoWriter_fourcc(*"mp4v"),
        fps,
        (width, height),
    )

    trail_points = []
    frame_idx = 0

    while True:
        ret, frame = cap.read()

        if not ret:
            break

        if court_points is not None:
            draw_court(frame, court_points)

        if frame_idx in preds:
            visible, x, y = preds[frame_idx]

            if visible == 1 and x > 0 and y > 0:
                trail_points.append((x, y))
                trail_points = trail_points[-args.trail:]

                for i in range(1, len(trail_points)):
                    cv2.line(
                        frame,
                        trail_points[i - 1],
                        trail_points[i],
                        (0, 255, 255),
                        2,
                    )

                cv2.circle(frame, (x, y), 10, (0, 0, 255), 3)
                cv2.circle(frame, (x, y), 3, (0, 0, 255), -1)

                label = f"Frame {frame_idx} | visible=1 | x={x}, y={y}"
            else:
                label = f"Frame {frame_idx} | visible=0"

            cv2.putText(
                frame,
                label,
                (30, 50),
                cv2.FONT_HERSHEY_SIMPLEX,
                1.0,
                (255, 255, 255),
                3,
            )
            cv2.putText(
                frame,
                label,
                (30, 50),
                cv2.FONT_HERSHEY_SIMPLEX,
                1.0,
                (0, 0, 0),
                1,
            )

        writer.write(frame)
        frame_idx += 1

    cap.release()
    writer.release()

    print(f"Saved checked video to: {output_path}")


if __name__ == "__main__":
    main()


# python scripts/check_shuttle.py 
# input.mp4
# --court_points outputs/input.txt
# --output outputs/input_checked.mp4

#python scripts/download_clip.py 
# "https://www.youtube.com/watch?v=ZNUbTBv5hXc&t=73s" 
# 00:02:15 00:02:25 -o input/multi_c_11.mp4

# external/court-detection/build/detect input/multi_c_11.mp4 
# outputs/court_points/multi_c_11.txt


# python scripts/check_shuttle.py input/multi_c_11.mp4 
# --court_points outputs/court_points/multi_c_11.txt --output outputs/multi_c_11_checked.mp4
