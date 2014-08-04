local class = require 'core/middleclass.lua'


local AudioSource = class('core/AudioSource')

function AudioSource.static:playAll()
    NATIVE.PlayAllAudioSources()
end

function AudioSource.static:pauseAll()
    NATIVE.PauseAllAudioSources()
end

function AudioSource:initialize( triggerCallback )
    self.handle = NATIVE.CreateAudioSource(triggerCallback or false)
end

function AudioSource:setRelative( relative )
    NATIVE.SetAudioSourceRelative(self.handle, relative)
end

function AudioSource:setLooping( looping )
    NATIVE.SetAudioSourceLooping(self.handle, looping)
end

function AudioSource:setPitch( pitch )
    NATIVE.SetAudioSourcePitch(self.handle, pitch)
end

function AudioSource:setGain( gain )
    NATIVE.SetAudioSourceGain(self.handle, gain)
end

function AudioSource:setAttachmentTarget( solid )
    NATIVE.SetAudioSourceAttachmentTarget(self.handle, solid.handle)
end

function AudioSource:setTransformation( matrix )
    NATIVE.SetAudioSourceTransformation(self.handle, matrix.handle)
end

function AudioSource:enqueue( buffer )
    NATIVE.EnqueueAudioBuffer(self.handle, buffer.handle)
end

function AudioSource:play()
    NATIVE.PlayAudioSource(self.handle)
end

function AudioSource:pause()
    NATIVE.PauseAudioSource(self.handle)
end

function AudioSource:free()
    NATIVE.FreeAudioSource(self.handle)
end


return AudioSource
