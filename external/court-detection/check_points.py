import cv2

img = cv2.imread("/Users/jhkim/Desktop/badminton_videos/second_frames/frame_0339.jpg")




points = [
(393.511,443.103),
(81.7386, 1062.69),
(1868.89, 1060.31),
(1541.02, 439.46),
(271.004, 443.076),
(1668.62, 440.045)
]

for i, (x, y) in enumerate(points, 1):
    cv2.circle(img, (round(x), round(y)), 12, (0, 0, 255), -1)
    cv2.putText(img, str(i), (round(x)+15, round(y)-15),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 0, 255), 3)

cv2.imwrite("court_points_overlay.jpg", img)
print("saved court_points_overlay.jpg")