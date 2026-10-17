
# Badminton Shot Feedback

Research prototype for event-centric badminton video understanding

## Current Model

Extract:
 - court geometry
 - shuttle trajectory

## Pipeline 

Video
-> Court Detection
-> Shuttle Tracking
-> Hit Event Extraction
-> Shot Logging
-> Tactical Analysis

## External Modules

 - [TrackNetV3](https://github.com/qaz812345/TrackNetV3) (shuttle tracking, MIT): downloaded separately; see setup below.
 - Court detection (BSD 3-Clause): source and local improvements are included in `external/court-detection/`; compiled build files are excluded. See its README for algorithm background and build instructions.

Original licenses are retained in the component directories and reproduced in [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).

## Restore TrackNetV3

The TrackNetV3 directory and model checkpoints are excluded from this repository.
From the project root, restore the version used here and apply the saved local changes:

```sh
git clone https://github.com/qaz812345/TrackNetV3.git external/TrackNetV3
git -C external/TrackNetV3 checkout 77c123ad4dd449b7d275f16cc43f316ba5b54042
git -C external/TrackNetV3 apply ../../patches/tracknetv3-local.patch
```

The patch preserves CPU inference and single-process data loading changes made for the local Mac setup, plus removal of the old OpenCV dependency pin. It does not improve the tracking model. Install a compatible OpenCV package separately and follow the TrackNetV3 README for dependencies and checkpoint downloads.

Inputs, generated outputs, and the Python virtual environment are excluded from version control.
 
