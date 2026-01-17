#!/usr/bin/env python3
"""
Bad Apple Video Converter for HuTaOS
Converts MP4 video (320x200 @ 30 fps) into a binary bitmap format
suitable for rendering on the OS display.

Output Format:
  - [frame_size] bytes    = raw 1-bit frame data (1 bit per pixel, 8 pixels per byte)

Each frame is stored as raw pixels in row-major order, 8 pixels packed per byte,
with the MSB representing the leftmost pixel.
"""

import cv2
import numpy as np
import struct
import sys
import os
from pathlib import Path


class VideoConverter:
    def __init__(self, video_path, output_path, width=320, height=200, fps=30):
        """
        Initialize the video converter.
        
        Args:
            video_path: Path to input MP4 video
            output_path: Path to output binary file
            width: Target frame width (default 320)
            height: Target frame height (default 200)
            fps: Target frames per second (default 30)
        """
        self.video_path = video_path
        self.output_path = output_path
        self.width = width
        self.height = height
        self.fps = fps
        self.frames_data = []
        
    def load_video(self):
        """Load video file and check properties."""
        if not os.path.exists(self.video_path):
            raise FileNotFoundError(f"Video file not found: {self.video_path}")
        
        cap = cv2.VideoCapture(self.video_path)
        if not cap.isOpened():
            raise RuntimeError(f"Failed to open video: {self.video_path}")
        
        # Get video properties
        frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        video_fps = cap.get(cv2.CAP_PROP_FPS)
        video_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        video_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        
        print(f"Video Properties:")
        print(f"  Resolution: {video_width}x{video_height}")
        print(f"  FPS: {video_fps}")
        print(f"  Frame Count: {frame_count}")
        print()
        
        return cap, frame_count, video_fps
    
    def frame_to_1bit_packed(self, frame):
        """
        Convert an OpenCV BGR frame to 1-bit black/white packed binary format.
        
        Dithering is applied for better quality conversion from color/grayscale.
        Each byte contains 8 pixels (MSB = left pixel, LSB = right pixel).
        
        Args:
            frame: OpenCV BGR frame (may be color or grayscale)
        
        Returns:
            bytes: Packed 1-bit frame data (width * height / 8 bytes)
        """
        # Convert BGR to grayscale
        if len(frame.shape) == 3:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        else:
            gray = frame
        
        # Resize to target dimensions
        resized = cv2.resize(gray, (self.width, self.height), interpolation=cv2.INTER_AREA)
        
        # Apply Otsu's thresholding for better black/white conversion
        _, binary = cv2.threshold(resized, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        
        # Convert to numpy array and invert (so white = 1, black = 0)
        # This matches the expected format
        pixels = (binary > 128).astype(np.uint8)
        
        # Pack 8 pixels per byte
        packed_data = bytearray()
        for row in range(self.height):
            for col in range(0, self.width, 8):
                byte_val = 0
                for bit in range(8):
                    if col + bit < self.width:
                        # MSB is leftmost pixel
                        if pixels[row, col + bit]:
                            byte_val |= (1 << (7 - bit))
                packed_data.append(byte_val)
        
        return bytes(packed_data)
    
    def convert(self):
        """Convert video to binary frame format."""
        cap, frame_count, video_fps = self.load_video()
        
        print(f"Converting video frames...")
        frame_idx = 0
        
        while True:
            ret, frame = cap.read()
            if not ret:
                break
            
            # Convert frame to 1-bit packed format
            frame_data = self.frame_to_1bit_packed(frame)
            self.frames_data.append(frame_data)
            
            # Progress indicator
            if (frame_idx + 1) % 30 == 0 or (frame_idx + 1) == frame_count:
                print(f"  Converted {frame_idx + 1}/{frame_count} frames")
            
            frame_idx += 1
        
        cap.release()
        
        if not self.frames_data:
            raise RuntimeError("No frames extracted from video")
        
        print(f"Successfully converted {len(self.frames_data)} frames")
        print()
    
    def calculate_frame_size(self):
        """Calculate size of a single frame in bytes."""
        return (self.width * self.height) // 8
    
    def write_binary(self):
        """Write frames to binary file with header."""
        frame_size = self.calculate_frame_size()
        
        with open(self.output_path, 'wb') as f:
            # Write header (16 bytes)
            for i, frame_data in enumerate(self.frames_data):
                f.write(frame_data)
                
                if (i + 1) % 30 == 0 or (i + 1) == len(self.frames_data):
                    print(f"  Wrote {i + 1}/{len(self.frames_data)} frames")
            
            file_size = os.path.getsize(self.output_path)
            print(f"\nBinary file written: {self.output_path}")
            print(f"File size: {file_size:,} bytes ({file_size / 1024 / 1024:.2f} MB)")
    
    def run(self):
        """Execute the conversion process."""
        try:
            print(f"Loading video: {self.video_path}")
            print()
            self.load_video()
            self.convert()
            self.write_binary()
            print("\nConversion completed successfully!")
            return True
        except Exception as e:
            print(f"Error: {e}")
            import traceback
            traceback.print_exc()
            return False


def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        print("Usage: python video-converter.py <input.mp4> [output.bin]")
        print()
        print("Converts an MP4 video to a binary bitmap format suitable for")
        print("rendering on HuTaOS (320x200 @ 30 fps, 1-bit black/white)")
        print()
        print("Arguments:")
        print("  input.mp4    - Input video file (320x200 @ 30 fps)")
        print("  output.bin   - Output binary file (default: output.bin)")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "output.bin"
    
    converter = VideoConverter(input_file, output_file)
    success = converter.run()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
