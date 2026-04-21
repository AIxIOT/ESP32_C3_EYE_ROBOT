#pragma once

enum EyeEmotion {
  EMOTION_NORMAL = 0,  // ปกติ - กรอกตาไปมา
  EMOTION_HAPPY,       // ดีใจ - ตาโค้งขึ้น ^_^
  EMOTION_ANGRY,       // โกรธ - ตาเฉียง
  EMOTION_CRAZY,       // บ้า - สั่น ขนาดเปลี่ยน
  EMOTION_SLEEPY,      // งีบ - ค่อยๆ หลับ
  EMOTION_HUNGRY,      // หิว - ตาปรือ + น้ำลายไหล
  EMOTION_SAD,         // เศร้า - ตาลู่ลง + น้ำตา
  EMOTION_LAUGH,       // หัวเราะ - ตาหยี ^▽^ + สั่นเด้ง
  EMOTION_COUNT
};

void eye_mode_init();
void eye_mode_update();
void eye_mode_set_emotion(EyeEmotion e);
EyeEmotion eye_mode_get_emotion();
const char* eye_mode_emotion_name();
