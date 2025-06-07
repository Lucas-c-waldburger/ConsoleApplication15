#pragma once
#include <array>
#include <vector>
#include "custom/CustomEventData.h"


// keeps a buffer around to store calls to SDL_PeepEvents, delivers them as a vector to caller
template <size_t N>
class SDLEventBufferTransformer
{
public:
    constexpr SDLEventBufferTransformer() noexcept
    {
        for (size_t i = 0; i < N; i++)
        {
            SDL_zero(buffer_[i]);
        }
    }

    std::vector<SDL_Event> RetrieveEventRange(SDL_eventaction action, uint32_t typeStart, uint32_t typeEnd)
    {
        assert(typeStart <= typeEnd && "Start event type is greater than end");

        int eventsRetrieved = SDL_PeepEvents(&buffer_[0], N, action, typeStart, typeEnd);
        if (eventsRetrieved <= 0)
        {
            if (eventsRetrieved < 0)
            {
                LOG_ERROR_FMT("Peep events failed: {}", SDL_GetError());
            }

            return {};
        }

        return std::vector<SDL_Event>(&buffer_[0], &buffer_[eventsRetrieved]);
    }

    template <typename...Ts> requires (std::convertible_to<Ts, uint32_t> && ...)
    std::vector<SDL_Event> RetrieveEventTypes(SDL_eventaction action, Ts...types)
    {
        int runningSize = 0;

        ((RetrieveEventTypeImpl(runningSize, buffer_, action, types)), ...);

        return std::vector<SDL_Event>{&buffer_[0], & buffer_[static_cast<size_t>(runningSize)]};
    }

private:
    static void RetrieveEventTypeImpl(int& runningSize, std::array<SDL_Event, N>& buffer,
                                      SDL_eventaction action, uint32_t evType)
    {
        assert(runningSize <= N);

        if (runningSize < 0 || runningSize == N)
        {
            return;
        }

        int eventsRetrieved = SDL_PeepEvents(&buffer[static_cast<size_t>(runningSize)],
                                             N - runningSize, action, evType, evType);

        if (eventsRetrieved < 0)
        {
            LOG_ERROR_FMT("Peep events failed: {}", SDL_GetError());
            runningSize = -1;
        }
        else
        {
            runningSize += eventsRetrieved;
        }
    }

    std::array<SDL_Event, N> buffer_;
};