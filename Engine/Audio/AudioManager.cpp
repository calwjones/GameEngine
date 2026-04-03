#include "AudioManager.h"

namespace Engine {

bool AudioManager::loadSound(const std::string& name, const std::string& path) {
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

void AudioManager::playSound(const std::string& name) {
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

bool AudioManager::playMusic(const std::string& path, bool loop) {
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

void AudioManager::stopMusic() {
#ifdef HAS_SFML_AUDIO
    m_music.stop();
#endif
}

void AudioManager::pauseMusic() {
#ifdef HAS_SFML_AUDIO
    m_music.pause();
#endif
}

void AudioManager::resumeMusic() {
#ifdef HAS_SFML_AUDIO
    if (m_music.getStatus() == sf::Music::Paused) m_music.play();
#endif
}

void AudioManager::setMusicVolume(float v) {
    m_musicVolume = v;
#ifdef HAS_SFML_AUDIO
    m_music.setVolume(v);
#endif
}

bool AudioManager::hasSound(const std::string& name) const {
#ifdef HAS_SFML_AUDIO
    return m_buffers.count(name) > 0;
#else
    (void)name;
    return false;
#endif
}

void AudioManager::stopAll() {
#ifdef HAS_SFML_AUDIO
    for (auto& s : m_sounds) s.stop();
    m_music.stop();
#endif
}

}
