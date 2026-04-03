#pragma once

#include <string>

#ifdef HAS_SFML_AUDIO
#include <SFML/Audio.hpp>
#include <unordered_map>
#endif

namespace Engine {

class AudioManager {
#ifdef HAS_SFML_AUDIO
    std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
    static constexpr int MAX_SOUNDS = 16;
    sf::Sound m_sounds[MAX_SOUNDS];
    int m_nextSound = 0;
    sf::Music m_music;
#endif
    float m_sfxVolume = 100.f;
    float m_musicVolume = 50.f;
    bool m_enabled = true;

public:
    bool loadSound(const std::string& name, const std::string& path);
    void playSound(const std::string& name);

    bool playMusic(const std::string& path, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();

    void setSfxVolume(float v) { m_sfxVolume = v; }
    void setMusicVolume(float v);
    float getSfxVolume() const { return m_sfxVolume; }
    float getMusicVolume() const { return m_musicVolume; }

    void setEnabled(bool e) { m_enabled = e; if (!e) stopAll(); }
    bool isEnabled() const { return m_enabled; }

    bool hasSound(const std::string& name) const;
    void stopAll();
};

}
