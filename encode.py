import pylab
import imageio
import skimage.transform

HEIGHT = 64
WIDTH = 128
FPS = 30

filename = 'bad_apple.mp4'
video = imageio.get_reader(filename, 'ffmpeg')


def get_frame(f):
    frame = video.get_data(f)
    frame = skimage.transform.resize(frame, (HEIGHT, WIDTH))

    # for y in range(HEIGHT):
    #     for x in range(WIDTH):
    #         print('#' if frame[y, x][0] > 0.5 else ' ', end='')
    #     print('')
    # print('\n')
    # print('\n')
    return frame


# fig = pylab.figure()
# fig.suptitle('image', fontsize=20)
# pylab.imshow(get_frame(100))
# pylab.show()


def encode_image(img):
    result = []

    for x in range(0, WIDTH, 8):
        for y in range(0, HEIGHT, 8):
            for yi in range(8):
                v = 0
                for xi in range(8):
                    v = (v << 1) | (1 if img[y + yi, x + xi][0] > 0.5 else 0)
                # (x * 8 + y) * 8 + i
                result.append(v)

    return result


def encode_video(frame_indices):
    data = []
    last = None

    for frame_counter, frame_index in enumerate(frame_indices):
        frame = encode_image(get_frame(frame_index))
        if len(data) == 0:
            print("processed first frame")
            # 255 = full frame
            data.append(255)
            data += frame
            last = frame
            print(len(frame))
            continue
        changes_frame = []

        # print("changes frame:")
        for i in range(0, len(last), 8):
            for j in range(8):
                if last[i + j] != frame[i + j]:
                    break
            else:
                continue
            changes_frame.append(i // 8)
            for j in range(8):
                changes_frame.append(frame[i + j])

        print(f"processed frame {frame_counter}/{len(frame_indices)}")
        if len(changes_frame) < len(frame):
            print("using changes frame", len(changes_frame) // 9)
            data.append(len(changes_frame) // 9)
            data += changes_frame
        else:
            print("using full frame")
            data.append(255)
            data += frame
        last = frame

    print(f"total compression: {len(data) / (len(frame_indices) * 8 * (WIDTH // 8) * (HEIGHT // 8))}")
    return data


if __name__ == '__main__':
    result_bytes = bytearray(encode_video(range(0, 3 * 60 * FPS, 2)))
    with open(filename + ".bin", "wb") as f:
         f.write(result_bytes)
