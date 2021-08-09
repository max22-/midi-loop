#include <iostream>
#include <SFML/Audio.hpp>
#include <portmidi.h>

enum class MSG {
    SET_START = 0x00C0,
    SET_END = 0x01C0,
    RESET_START = 0x2C0,
    RESET_END = 0x03C0,
    PLAY_PAUSE = 0x04C0,
    REWIND = 0x05C0
};

void midi_devices_list(uint8_t midi_device)
{
    std::cout << "Midi devices :" << std::endl;
    for(int i = 0; i < Pm_CountDevices(); i++) {
        const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(i);
        if(deviceInfo->input)
            std::cout << "[" << (i == midi_device ? "x" : " ") << "]" << " #" << i << " -> " << deviceInfo->name << std::endl;
    }
}

void usage()
{
    std::cout << "Usage : midi-loop soundfile.ogg midi-device" << std::endl;
}

int main(int argc, char *argv[])
{
    uint8_t midi_device = -1;

    Pm_Initialize();

    if(argc != 3) {
        usage();
        midi_devices_list(midi_device);
        return(EXIT_FAILURE);
    }

    std::cout << "midi-loop" << std::endl;
    midi_device = atoi(argv[2]);
    midi_devices_list(midi_device);

    PmStream *midi;
    Pm_OpenInput(&midi, midi_device, nullptr, 128, nullptr, nullptr);
    if(midi == nullptr) {
        std::cerr << "Failed to open midi device #" << midi_device << std::endl;
        return EXIT_FAILURE;
    }

    sf::Music music;
    if(!music.openFromFile(argv[1]))
        return EXIT_FAILURE;

    music.play();
    music.setLoop(true);

    sf::Music::TimeSpan music_loop(sf::seconds(0), music.getDuration());

    while(true) {
        if(Pm_Poll(midi) == pmGotData) {
            PmEvent buffer[1];
            int events = Pm_Read(midi, buffer, sizeof(buffer) / sizeof(PmEvent));
            for(int i = 0; i < events; i++)
                std::cout << std::hex << buffer[i].message << " ";
            std::cout << std::endl;
            switch((MSG)buffer[0].message) {
                case MSG::SET_START:
                    music_loop.length -= music.getPlayingOffset() - music_loop.offset;
                    music_loop.offset = music.getPlayingOffset();
                    music.setLoopPoints(music_loop);
                    break;
                case MSG::SET_END:
                    music_loop.length = music.getPlayingOffset() - music_loop.offset;
                    music.setLoopPoints(music_loop);
                    break;
                case MSG::RESET_START:
                    music_loop.length += music_loop.offset;
                    music_loop.offset = sf::seconds(0);
                    music.setLoopPoints(music_loop);
                    break;
                case MSG::RESET_END:
                    music_loop.length = music.getDuration() - music_loop.offset;
                    music.setLoopPoints(music_loop);
                    break;
                case MSG::PLAY_PAUSE:
                    if(music.getStatus() != sf::Music::Playing)
                        music.play();
                    else
                        music.pause();
                    break;
                case MSG::REWIND:
                    music.setPlayingOffset(music_loop.offset);
                    break;
                default:
                    break;
            }
        }
    }

    Pm_Close(midi);
    return EXIT_SUCCESS;
}
