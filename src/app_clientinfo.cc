/*
Copyright (c) 2017, rpi-webrtc-streamer Lyu,KeunChang

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "app_clientinfo.h"

static const uint32_t kClientIDLength = 8;
static const uint32_t kRoomIDLength = 9;
static const uint32_t kMaxClientID     = 99999999;
static const uint32_t kMaxRoomID = 999999999;
static const uint64_t kWaitTimeout = 1000;

//////////////////////////////////////////////////////////////////////////////////////////
//
// AppClientInfo
//
//////////////////////////////////////////////////////////////////////////////////////////
AppClientInfo::AppClientInfo () : clock_(webrtc::Clock::GetRealTimeClock()), 
    last_wait_timestamp_(0), state_(ClientState::CLIENT_DISCONNECTED) {
}

bool AppClientInfo::ConnectWait (int room_id, int& client_id)  {
    int min, max;
    uint64_t timestamp_diff;
    LOG(LS_VERBOSE) << __FUNCTION__ << ": Roomid " << room_id
        << ",Client id: " << client_id;

    // Validate Client status 
    switch( state_ ) {
        case ClientState::CLIENT_CONNECT_WAIT:
            timestamp_diff = clock_->TimeInMilliseconds() - last_wait_timestamp_;
            // checking connect wait timeout
            if( timestamp_diff < kWaitTimeout ) {
                LOG(LS_VERBOSE) << "Rejecting CONNECT_WAIT during WAIT state";
                return false;   
            };
            break;
        case ClientState::CLIENT_CONNECTED:
            // Room already occupied by another client
            return false;
        case ClientState::CLIENT_DISCONNECTED:
        case ClientState::CLIENT_DISCONNECT_WAIT:
        default:
            break;
    };

    state_ = CLIENT_CONNECT_WAIT;

    // Generate new random client id
    min = kMaxClientID / 10;
    max = kMaxClientID;
    client_id_ = client_id = min + (rtc::CreateRandomId() % (int)(max - min + 1));
    last_wait_timestamp_ = clock_->TimeInMilliseconds();

    room_id_ = room_id;
    return true;
}

bool AppClientInfo::Connected (int websocket_id, int room_id, int client_id)  {
    LOG(LS_VERBOSE) << __FUNCTION__ << "WS id: " << websocket_id
        << ", Roomid " << room_id << ",Client id: " << client_id;
    RTC_DCHECK( websocket_id >= 0 );
    RTC_DCHECK( room_id >= 0 );
    RTC_DCHECK( client_id >= 0 );
    switch( state_ ) {
        case ClientState::CLIENT_CONNECTED:
            // should not be happend
            RTC_DCHECK( room_id_ == room_id && client_id_ == client_id );
            return false;
        // All other state can be changed to CONNECTED
        case ClientState::CLIENT_DISCONNECTED:
        case ClientState::CLIENT_DISCONNECT_WAIT:
            room_id_ = room_id;
            client_id_  = client_id;
            break;
        case ClientState::CLIENT_CONNECT_WAIT:
        default:
            break;
    }
    RTC_DCHECK( room_id_ == room_id );
    websocket_id_ = websocket_id;

    // this type of request generated by testing page of browser
    // TODO: Need to check remote ip is private ip address
    if( room_id_ == 0 && client_id_ == 0 ) {
        room_id_ = room_id; client_id_ = client_id;
    } 

    state_ = CLIENT_CONNECTED;
    return true;
}

bool AppClientInfo::DisconnectWait(int room_id, int client_id)  {
    LOG(LS_VERBOSE) << __FUNCTION__ << ": Roomid " << room_id 
        << ",Client id: " << client_id;
    RTC_DCHECK( room_id_ == room_id );
    RTC_DCHECK( client_id_ == client_id );
    switch( state_ ) {
        case ClientState::CLIENT_CONNECTED:
            state_ = ClientState::CLIENT_DISCONNECT_WAIT;
            return true;
        default:
            break;
    }
    return true;
}

bool AppClientInfo::DisconnectWait(int websocket_id )  {
    LOG(LS_VERBOSE) << __FUNCTION__ << ": Websocket id " << websocket_id ;
    RTC_DCHECK( websocket_id_ == websocket_id );
    switch( state_ ) {
        case ClientState::CLIENT_CONNECTED:
            state_ = ClientState::CLIENT_DISCONNECT_WAIT;
            return true;
        default:
            break;
    }
    return true;
}

bool AppClientInfo::GetWebsocketId(int client_id, int& websocket_id)  {
    LOG(LS_VERBOSE) << __FUNCTION__ << ":Client id: " << client_id;
    RTC_DCHECK( client_id_ == client_id );
    if( client_id_ == client_id ) {
        websocket_id = websocket_id_;
        return true;
    }
    return false;
}

bool AppClientInfo::IsConnected (int room_id, int client_id)  {
    if( (state_ == ClientState::CLIENT_CONNECTED ||
            state_ == ClientState::CLIENT_CONNECT_WAIT ) &&
        room_id == room_id_ && client_id == client_id_ )
        return true;
    return false;
}


bool AppClientInfo::IsConnected (int websocket_id)  {
    if( (state_ == ClientState::CLIENT_CONNECTED ||
            state_ == ClientState::CLIENT_CONNECT_WAIT ) &&
        websocket_id == websocket_id_  )
        return true;
    return false;
}

void AppClientInfo::Reset()  {
    LOG(LS_VERBOSE) << __FUNCTION__ ;
    client_id_ = room_id_ = websocket_id_ = 0;
    state_ =  CLIENT_DISCONNECTED;
}

