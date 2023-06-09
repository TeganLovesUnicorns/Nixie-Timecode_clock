#include <Arduino.h>
#include <smpte_ltc.h>

// Constructors
Timecode::Timecode()
    : _hours(0), _minutes(0), _seconds(0), _frames(0), _frameRate(30.0),
      _dropFrame(false) {}

Timecode::Timecode(unsigned int h, unsigned int m, unsigned int s,
                   unsigned int fr, double f, bool b)
    : _hours(h), _minutes(m), _seconds(s), _frames(fr), _frameRate(f),
      _dropFrame(b) {
  _validate();
}

Timecode::Timecode(uint_fast32_t frameInput, double f, bool b)
    : _frameRate(f), _dropFrame(b) {
  int nominal_fps = _nominalFramerate();
  int dropCount = nominal_fps / 15;
  int frameLimit = _maxFrames();
  int framesPerMin = 60 * nominal_fps;
  int framesPer10Min = framesPerMin * 10;
  int framesPerHour = framesPer10Min * 6;

  if (frameInput >= frameLimit) {
    frameInput %= frameLimit;
  }

  if (_dropFrame) {
    framesPerMin -= (nominal_fps / 15);
    framesPer10Min = framesPerMin * 10 + dropCount;
    framesPerHour = framesPer10Min * 6;
  }

  _hours = frameInput / framesPerHour;
  frameInput %= framesPerHour;

  if (_dropFrame) {
    // By subtracting two frames while calculating minutes and adding them
    // back in afterward, you avoid landing on an invalid dropframe frame
    // (frame 0 on any minute except every tenth minute)
    int ten_minute = frameInput / framesPer10Min;
    frameInput %= framesPer10Min;
    frameInput -= dropCount;
    int unit_minute = frameInput / framesPerMin;
    frameInput %= framesPerMin;
    _minutes = ten_minute * 10 + unit_minute;
    frameInput += dropCount;
  } else {
    _minutes = (frameInput / framesPer10Min);
    frameInput %= framesPer10Min;
    _minutes += frameInput / framesPerMin;
    frameInput %= framesPerMin;
  }

  _seconds = frameInput / nominal_fps;
  frameInput %= nominal_fps;

  _frames = frameInput;
}

Timecode::Timecode(const std::string &s, const float &f, const bool &b)
    : _frameRate(f), _dropFrame(b) {
  _setTimecode(s.c_str());
  _validate();
}

// Static member functions
std::string Timecode::framesToString(uint_fast32_t frames, double framerate,
                                     bool dropframe) {
  Timecode tc(frames, framerate, dropframe);
  return tc.to_string();
}

// Private member functions

void Timecode::_setTimecode(const char *c) {
  uint_fast16_t h, m, s, f;

  if (sscanf(c, "%2hu%*1c%2hu%*1c%2hu%*1c%2hu", &h, &m, &s, &f) == 4) {
    _hours = h;
    _minutes = m;
    _seconds = s;
    _frames = f;
  } else {
    throw(std::invalid_argument("Invalid string passed."));
  }
}

uint_fast16_t Timecode::_nominalFramerate() const {
  int nominal_fps = static_cast<int>(_frameRate);

  // nominal fps for 29.97 is 30, for 59.94 is 60, for 23.98 is 24
  switch (nominal_fps) {
  case 29:
  case 23:
  case 59:
    return ++nominal_fps;
  default:
    return nominal_fps;
  }
}

uint_fast32_t Timecode::_maxFrames() const {
  // 1 less than frameRate times 60 seconds, 60 minutes, 24 hours
  Timecode t(23, 59, 59, _nominalFramerate() - 1, _frameRate, _dropFrame);

  return t.totalFrames();
}

void Timecode::_validate() {
  if (_frames >= _nominalFramerate()) {
    _frames %= _nominalFramerate();
    _seconds++;
  }
  if (_seconds > 59) {
    _seconds %= 60;
    _minutes++;
  }
  if (_minutes > 59) {
    _minutes %= 60;
    _hours++;
  }
  if (_hours > 23) {
    _hours %= 24;
  }
  if (_dropFrame && _frames == 0 && _seconds == 0 && _minutes % 10 != 0) {
    _frames += 2;
  }
};

// Getters

unsigned int Timecode::hours() const { return _hours; }
unsigned int Timecode::minutes() const { return _minutes; }
unsigned int Timecode::seconds() const { return _seconds; }
unsigned int Timecode::frames() const { return _frames; }
float Timecode::framerate() const { return _frameRate; }
bool Timecode::dropframe() const { return _dropFrame; }
char Timecode::_separator() const { return _dropFrame ? ';' : ':'; }

uint_fast32_t Timecode::totalFrames() const {
  int nominal_fps = _nominalFramerate();
  int framesPerMin = 60 * nominal_fps;
  int framesPer10Min = framesPerMin * 10;
  int framesPerHour = framesPer10Min * 6;
  int totalFrames = 0;

  if (_dropFrame) {
    int dropCount = nominal_fps / 15;
    framesPerMin -= dropCount;
    framesPer10Min = (framesPerMin * 10) + dropCount;
    framesPerHour = framesPer10Min * 6;
  }

  totalFrames += _hours * framesPerHour;
  totalFrames += (_minutes % 10) * framesPerMin;
  totalFrames += (_minutes / 10) * framesPer10Min;
  totalFrames += _seconds * nominal_fps;
  totalFrames += _frames;

  return totalFrames;
}

// Setters
void Timecode::hours(const unsigned int &h) { _hours = h; }
void Timecode::minutes(const unsigned int &m) { _minutes = m; }
void Timecode::seconds(const unsigned int &s) { _seconds = s; }
void Timecode::frames(const unsigned int &f) { _frames = f; }
void Timecode::framerate(const float &f) { _frameRate = f; }
void Timecode::dropframe(const bool &b) { _dropFrame = b; }

// Type conversion
std::string Timecode::to_string() const {
  char *c = new char[12];
  sprintf(c, "%02d:%02d:%02d%c%02d", _hours, _minutes, _seconds, _separator(),
          _frames);
  std::string s(c);
  delete[] c;
  return s;
}

Timecode::operator int() const { return totalFrames(); }

// Operators
Timecode Timecode::operator+(const Timecode &t) const {
  return operator+(t.totalFrames());
}

Timecode Timecode::operator+(const int &i) const {
  int frameSum = totalFrames() + i;
  frameSum %= (_maxFrames() + 1);
  return Timecode(frameSum, _frameRate, _dropFrame);
}

Timecode Timecode::operator-(const Timecode &t) const {
  return operator-(t.totalFrames());
}

Timecode Timecode::operator-(const int &i) const {
  int_fast32_t f = totalFrames() - i;
  if (f < 0)
    f += _maxFrames();
  return Timecode(f, _frameRate, _dropFrame);
}

Timecode Timecode::operator*(const int &i) const {
  int_fast32_t f = totalFrames() * i;
  while (f < 0)
    f += _maxFrames();
  return Timecode(f, _frameRate, _dropFrame);
}

bool Timecode::operator==(const Timecode &t) const {
  return totalFrames() == t.totalFrames();
}

bool Timecode::operator!=(const Timecode &t) const { return !(*this == t); }

bool Timecode::operator<(const Timecode &t) const {
  return totalFrames() < t.totalFrames();
}

bool Timecode::operator<=(const Timecode &t) const {
  return (*this < t || *this == t);
}

bool Timecode::operator>(const Timecode &t) const {
  return totalFrames() > t.totalFrames();
}

bool Timecode::operator>=(const Timecode &t) const {
  return (*this > t || *this == t);
}

std::ostream &operator<<(std::ostream &out, const Timecode &t) {
  out << t.to_string();
  return out;
}