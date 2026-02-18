#pragma once

// SFML audio wrapper. two things to know:
//
// 1) SOUND POOL. sf::Sound can only play one clip at a time and if u let it go
//    out of scope mid-playback it cuts off. so i keep a fixed pool of 16 and
//    rotate through them with m_nextSound. if u fire >16 sounds in <1 sec the
//    oldest gets stomped. 16 is wayyy more than we ever need tho.
//
// 2) HAS_SFML_AUDIO guard. sfml-audio is optional on macOS (its a separate
//    Homebrew keg + openAL) so CMake defines this only if it finds it. if not,
//    every method becomes a no-op — u can still build + run the engine just
//    silently. important: i use ONE class declaration with #ifdef around the
//    bodies, not two different class decls. that way the silent + loud builds
//    cant drift apart in their public API

#include <string>

#ifdef HAS_SFML_AUDIO
#include <SFML/Audio.hpp>
#include <unordered_map>
#endif

namespace Engine {

class AudioManager {
#ifdef HAS_SFML_AUDIO
    std::unordered_map<std::string, sf::SoundBuffer> m_buffers;  // load once, key by name
    static constexpr int MAX_SOUNDS = 16;    // max overlapping sounds. more = just replaces oldest
    sf::Sound m_sounds[MAX_SOUNDS];
    int m_nextSound = 0;                     // circular pool index
    sf::Music m_music;                       // music is streamed not loaded, one at a time
#endif
    float m_sfxVolume = 100.f;
    float m_musicVolume = 50.f;
    bool m_enabled = true;

public:
    bool loadSound(const std::string& name, const std::string& path) {
#ifdef HAS_SFML_AUDIO
        sf::SoundBuffer buf;
        if (!buf.loadFromFile(path)) return false;
        m_buffers[name] = std::move(buf);
        return true;
#else
        (void)name; (void)path;
        return false;
#endif
    }

    // fire and forget. grabs the next free-ish slot in the pool, sets the buffer
    // on it, plays it, rolls the index. if u call this 17 times in a row the
    // first sound gets interrupted by the 17th. not a problem in practice
    void playSound(const std::string& name) {
#ifdef HAS_SFML_AUDIO
        if (!m_enabled) return;
        auto it = m_buffers.find(name);
        if (it == m_buffers.end()) return;

        m_sounds[m_nextSound].setBuffer(it->second);
        m_sounds[m_nextSound].setVolume(m_sfxVolume);
        m_sounds[m_nextSound].play();
        m_nextSound = (m_nextSound + 1) % MAX_SOUNDS;
#else
        (void)name;
#endif
    }

    bool playMusic(const std::string& path, bool loop = true) {
#ifdef HAS_SFML_AUDIO
        if (!m_enabled) return false;
        if (!m_music.openFromFile(path)) return false;
        m_music.setLoop(loop);
        m_music.setVolume(m_musicVolume);
        m_music.play();
        return true;
#else
        (void)path; (void)loop;
        return false;
#endif
    }

    void stopMusic() {
#ifdef HAS_SFML_AUDIO
        m_music.stop();
#endif
    }

    void pauseMusic() {
#ifdef HAS_SFML_AUDIO
        m_music.pause();
#endif
    }

    void resumeMusic() {
#ifdef HAS_SFML_AUDIO
        if (m_music.getStatus() == sf::Music::Paused) m_music.play();
#endif
    }

    void setSfxVolume(float v) { m_sfxVolume = v; }
    void setMusicVolume(float v) {
        m_musicVolume = v;
#ifdef HAS_SFML_AUDIO
        m_music.setVolume(v);
#endif
    }
    float getSfxVolume() const { return m_sfxVolume; }
    float getMusicVolume() const { return m_musicVolume; }

    void setEnabled(bool e) { m_enabled = e; if (!e) stopAll(); }
    bool isEnabled() const { return m_enabled; }

    bool hasSound(const std::string& name) const {
#ifdef HAS_SFML_AUDIO
        return m_buffers.count(name) > 0;
#else
        (void)name;
        return false;
#endif
    }

    void stopAll() {
#ifdef HAS_SFML_AUDIO
        for (auto& s : m_sounds) s.stop();
        m_music.stop();
#endif
    }
};

}
