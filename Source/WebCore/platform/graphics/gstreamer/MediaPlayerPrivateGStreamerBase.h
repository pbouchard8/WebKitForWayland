/*
 * Copyright (C) 2007, 2009 Apple Inc.  All rights reserved.
 * Copyright (C) 2007 Collabora Ltd. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2009, 2010 Igalia S.L
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * aint with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MediaPlayerPrivateGStreamerBase_h
#define MediaPlayerPrivateGStreamerBase_h
#if ENABLE(VIDEO) && USE(GSTREAMER)

#include "GRefPtrGStreamer.h"
#include "MainThreadNotifier.h"
#include "MediaPlayerPrivate.h"
#include "PlatformLayer.h"
#include "TextureMapperGL.h"
#include "TextureMapperPlatformLayer.h"
#include "TextureMapperPlatformLayerProxy.h"
#include <glib.h>
#include <wtf/Condition.h>
#include <wtf/Forward.h>
#include <wtf/RunLoop.h>

typedef struct _GstMessage GstMessage;
typedef struct _GstStreamVolume GstStreamVolume;
typedef struct _GstVideoInfo GstVideoInfo;
typedef struct _GstGLContext GstGLContext;
typedef struct _GstGLDisplay GstGLDisplay;

namespace WebCore {

class BitmapTextureGL;
class GraphicsContext;
class GraphicsContext3D;
class IntSize;
class IntRect;

#if USE(DXDRM)
class DiscretixSession;
#elif USE(PLAYREADY)
class PlayreadySession;
#endif

void registerWebKitGStreamerElements();

class MediaPlayerPrivateGStreamerBase : public MediaPlayerPrivateInterface
#if USE(COORDINATED_GRAPHICS_THREADED) || (USE(TEXTURE_MAPPER_GL) && !USE(COORDINATED_GRAPHICS))
    , public PlatformLayer
#endif
{

public:
    enum VideoSourceRotation {
        NoVideoSourceRotation,
        VideoSourceRotation90,
        VideoSourceRotation180,
        VideoSourceRotation270
    };

    virtual ~MediaPlayerPrivateGStreamerBase();

    FloatSize naturalSize() const override;

    void setVolume(float) override;
#if PLATFORM(WPE)
    float volume() const override;
#endif

#if USE(GSTREAMER_GL)
    bool ensureGstGLContext();
#endif

    bool supportsMuting() const override { return true; }
    void setMuted(bool) override;
    bool muted() const;

    MediaPlayer::NetworkState networkState() const override;
    MediaPlayer::ReadyState readyState() const override;

    bool ended() const override { return m_isEndReached; }

    void setVisible(bool) override { }
    void setSize(const IntSize&) override;
    void setPosition(const IntPoint&) override;
    void sizeChanged();

    void triggerDrain();

    void triggerRepaint(GstSample*);
    void repaint();

    void paint(GraphicsContext&, const FloatRect&) override;

    bool hasSingleSecurityOrigin() const override { return true; }
    virtual float maxTimeLoaded() const { return 0.0; }

    bool supportsFullscreen() const override;
    PlatformMedia platformMedia() const override;

    MediaPlayer::MovieLoadType movieLoadType() const override;
    virtual bool isLiveStream() const = 0;

    MediaPlayer* mediaPlayer() const { return m_player; }

    unsigned decodedFrameCount() const override;
    unsigned droppedFrameCount() const override;
    unsigned audioDecodedByteCount() const override;
    unsigned videoDecodedByteCount() const override;

#if USE(TEXTURE_MAPPER_GL) && !USE(COORDINATED_GRAPHICS)
    PlatformLayer* platformLayer() const override { return const_cast<MediaPlayerPrivateGStreamerBase*>(this); }
#if PLATFORM(WIN_CAIRO)
    // FIXME: Accelerated rendering has not been implemented for WinCairo yet.
    bool supportsAcceleratedRendering() const override { return false; }
#else
    bool supportsAcceleratedRendering() const override { return true; }
#endif
    void paintToTextureMapper(TextureMapper&, const FloatRect&, const TransformationMatrix&, float) override;
#endif

#if USE(COORDINATED_GRAPHICS_THREADED)
    PlatformLayer* platformLayer() const override { return const_cast<MediaPlayerPrivateGStreamerBase*>(this); }
    bool supportsAcceleratedRendering() const override { return true; }
#endif

#if ENABLE(ENCRYPTED_MEDIA)
    MediaPlayer::MediaKeyException addKey(const String&, const unsigned char*, unsigned, const unsigned char*, unsigned, const String&);
    MediaPlayer::MediaKeyException generateKeyRequest(const String&, const unsigned char*, unsigned);
    MediaPlayer::MediaKeyException cancelKeyRequest(const String&, const String&);
    void needKey(const String&, const String&, const unsigned char*, unsigned);
#endif

#if ENABLE(ENCRYPTED_MEDIA_V2)
    void needKey(RefPtr<Uint8Array> initData);
    void setCDMSession(CDMSession*);
    void keyAdded();
#endif

#if ENABLE(ENCRYPTED_MEDIA) || ENABLE(ENCRYPTED_MEDIA_V2)
    virtual void dispatchDecryptionKey(GstBuffer*);
#endif

#if USE(DXDRM) || USE(PLAYREADY)
#if USE(DXDRM)
    DiscretixSession* dxdrmSession() const;
#elif USE(PLAYREADY)
    PlayreadySession* prSession() const;
#endif
    virtual void emitSession();
#endif

    static bool supportsKeySystem(const String& keySystem, const String& mimeType);
    static MediaPlayer::SupportsType extendedSupportsType(const MediaEngineSupportParameters& parameters, MediaPlayer::SupportsType);

    GstElement* pipeline() const { return m_pipeline.get(); }

    void setVideoSourceRotation(VideoSourceRotation rotation);

#if USE(GSTREAMER_GL)
    NativeImagePtr nativeImageForCurrentTime() override;
#endif

protected:
    MediaPlayerPrivateGStreamerBase(MediaPlayer*);

#if !USE(HOLE_PUNCH_GSTREAMER)
    virtual GstElement* createVideoSink();
#endif

    void setStreamVolumeElement(GstStreamVolume*);
    virtual GstElement* createAudioSink() { return 0; }
    virtual GstElement* audioSink() const { return 0; }

    void setPipeline(GstElement*);
    void clearSamples();

    virtual bool handleSyncMessage(GstMessage*);

    void notifyPlayerOfVolumeChange();
    void notifyPlayerOfMute();

    static void volumeChangedCallback(MediaPlayerPrivateGStreamerBase*);
    static void muteChangedCallback(MediaPlayerPrivateGStreamerBase*);

    enum MainThreadNotification {
        VideoChanged = 1 << 0,
        VideoCapsChanged = 1 << 1,
        AudioChanged = 1 << 2,
        VolumeChanged = 1 << 3,
        MuteChanged = 1 << 4,
#if ENABLE(VIDEO_TRACK)
        TextChanged = 1 << 5,
#endif
    };

    MainThreadNotifier<MainThreadNotification> m_notifier;
    MediaPlayer* m_player;
    GRefPtr<GstElement> m_pipeline;
    GRefPtr<GstStreamVolume> m_volumeElement;
    GRefPtr<GstElement> m_videoSink;
    GRefPtr<GstElement> m_fpsSink;
    MediaPlayer::ReadyState m_readyState;
    mutable MediaPlayer::NetworkState m_networkState;
    mutable bool m_isEndReached;
    IntSize m_size;
    IntPoint m_position;
    mutable GMutex m_sampleMutex;
    GRefPtr<GstSample> m_sample;
#if USE(GSTREAMER_GL)
    RunLoop::Timer<MediaPlayerPrivateGStreamerBase> m_drawTimer;
#endif
    unsigned long m_repaintHandler;
    unsigned long m_drainHandler;
    mutable FloatSize m_videoSize;
    bool m_usingFallbackVideoSink;
#if USE(TEXTURE_MAPPER_GL) && !USE(COORDINATED_GRAPHICS_MULTIPROCESS)
    guint m_orientation;
    void updateTexture(BitmapTextureGL&, GstVideoInfo&);
#endif
#if USE(GSTREAMER_GL)
    GRefPtr<GstGLContext> m_glContext;
    GRefPtr<GstGLDisplay> m_glDisplay;
#endif

#if USE(COORDINATED_GRAPHICS_THREADED)
    RefPtr<TextureMapperPlatformLayerProxy> proxy() const override { return m_platformLayerProxy.copyRef(); }
    void swapBuffersIfNeeded() override { };
    void pushTextureToCompositor();
    RefPtr<TextureMapperPlatformLayerProxy> m_platformLayerProxy;
#endif

#if USE(GSTREAMER_GL) || USE(COORDINATED_GRAPHICS_THREADED)
    RefPtr<GraphicsContext3D> m_context3D;
    Condition m_drawCondition;
    Lock m_drawMutex;
#endif

private:

#if USE(WESTEROS_SINK) || USE(FUSION_SINK)
    void updateVideoRectangle();
#endif

#if ENABLE(ENCRYPTED_MEDIA)
#if USE(DXDRM)
    DiscretixSession* m_dxdrmSession;
#elif USE(PLAYREADY)
    PlayreadySession* m_prSession;
#endif
#endif

#if ENABLE(ENCRYPTED_MEDIA_V2)
    std::unique_ptr<CDMSession> createSession(const String&, CDMSessionClient*);
    CDMSession* m_cdmSession;
#endif
    VideoSourceRotation m_videoSourceRotation;
#if USE(TEXTURE_MAPPER_GL)
    TextureMapperGL::Flags m_textureMapperRotationFlag ;
#endif

};
}

#endif // USE(GSTREAMER)
#endif
