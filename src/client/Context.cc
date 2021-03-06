/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file client/Context.cc
 */

#include <client/Context.hh>

#include <client/Caelum.hh>
#include <client/Terra.hh>

#include <client/SMMImago.hh>
#include <client/SMMVehicleImago.hh>
#include <client/ExplosionImago.hh>
#include <client/MD2Imago.hh>
#include <client/MD2WeaponImago.hh>

#include <client/BasicAudio.hh>
#include <client/BotAudio.hh>
#include <client/VehicleAudio.hh>

#include <client/eSpeak.hh>

#define OZ_REGISTER_IMAGOCLASS(name) \
  { \
    int id = liber.imagoIndex(#name); \
    if (id >= 0) { \
      imagoClasses[id] = name##Imago::create; \
    } \
  }

#define OZ_REGISTER_AUDIOCLASS(name) \
  { \
    int id = liber.audioIndex(#name); \
    if (id >= 0) { \
      audioClasses[id] = name##Audio::create; \
    } \
  }

namespace oz
{
namespace client
{

struct Context::TextureResource::PreloadData
{
  Stream albedoBuffer;
  Stream masksBuffer;
  Stream normalsBuffer;
};

struct Context::SoundResource::PreloadData
{
  Stream      fileBuffer;
  AudioBuffer audioBuffer;
};

Pool<Context::Source> Context::Source::pool;
int                   Context::speakSampleRate;
Context::SpeakSource  Context::speakSource;

int Context::speakCallback(short int* samples, int nSamples, void*)
{
  if (nSamples != 0) {
    int maxSamples = SpeakSource::BUFFER_SIZE - speakSource.nSamples;
    if (nSamples > maxSamples) {
      nSamples = maxSamples;
      Log::printRaw("AL: Speak buffer overrun\n");
    }

    memcpy(speakSource.samples + speakSource.nSamples, samples, nSamples * sizeof(short));
    speakSource.nSamples += nSamples;

    if (speakSource.nQueuedBuffers != 2) {
      int i = speakSource.nQueuedBuffers;

      speakSource.mutex.lock();

      alBufferData(speakSource.bufferIds[i], AL_FORMAT_MONO16, speakSource.samples,
                   speakSource.nSamples * sizeof(short), speakSampleRate);
      alSourceQueueBuffers(speakSource.id, 1, &speakSource.bufferIds[i]);
      alSourcePlay(speakSource.id);

      OZ_AL_CHECK_ERROR();
      speakSource.mutex.unlock();

      ++speakSource.nQueuedBuffers;
      speakSource.nSamples = 0;
      return 0;
    }
  }

  do {
    if (speakSource.nQueuedBuffers == 0) {
      return 1;
    }

    int nProcessed;
    do {
      Time::sleep(100);

      speakSource.mutex.lock();

      int state;
      alGetSourcei(speakSource.id, AL_SOURCE_STATE, &state);
      alGetSourcei(speakSource.id, AL_BUFFERS_PROCESSED, &nProcessed);

      if (nProcessed == 0 && state == AL_STOPPED) {
        alSourcePlay(speakSource.id);
      }

      speakSource.mutex.unlock();
    }
    while (nProcessed == 0 && speakSource.isAlive);

    if (!speakSource.isAlive) {
      return 1;
    }

    speakSource.mutex.lock();

    ALuint buffer;
    alSourceUnqueueBuffers(speakSource.id, 1, &buffer);
    --speakSource.nQueuedBuffers;

    if (speakSource.nSamples != 0) {
      alBufferData(buffer, AL_FORMAT_MONO16, speakSource.samples,
                   speakSource.nSamples * sizeof(short), speakSampleRate);
      alSourceQueueBuffers(speakSource.id, 1, &buffer);

      ++speakSource.nQueuedBuffers;
      speakSource.nSamples = 0;
    }

    OZ_AL_CHECK_ERROR();
    speakSource.mutex.unlock();
  }
  while (nSamples == 0);

  return 0;
}

void Context::speakMain(void*)
{
  espeak_Synth(speakSource.text, speakSource.text.length(), 0, POS_CHARACTER, 0, espeakCHARS_UTF8,
               nullptr, nullptr);

  int value = AL_PLAYING;
  while (speakSource.isAlive && value != AL_STOPPED) {
    Time::sleep(100);

    speakSource.mutex.lock();
    alGetSourcei(speakSource.id, AL_SOURCE_STATE, &value);
    OZ_AL_CHECK_ERROR();
    speakSource.mutex.unlock();
  }

  speakSource.owner   = -1;
  speakSource.isAlive = false;
}

uint Context::addSource(int sound)
{
  hard_assert(sounds[sound].nUsers > 0);

  uint srcId;
  alGenSources(1, &srcId);

  // Can this ever happen?
  if (srcId == INVALID_SOURCE) {
    soft_assert(false);

    alDeleteSources(1, &srcId);
    return INVALID_SOURCE;
  }
  if (alGetError() != AL_NO_ERROR) {
    Log::printRaw("AL: Too many sources\n");
    return INVALID_SOURCE;
  }

  alSourcei(srcId, AL_BUFFER, sounds[sound].handle);

  ++sounds[sound].nUsers;
  sources.add(new Source(srcId, sound));
  return srcId;
}

void Context::removeSource(Source* source, Source* prev)
{
  int sound = source->sound;

  hard_assert(sounds[sound].nUsers > 0);

  alDeleteSources(1, &source->id);

  --sounds[sound].nUsers;
  sources.erase(source, prev);
  delete source;
}

uint Context::addContSource(int sound, int key)
{
  hard_assert(sounds[sound].nUsers > 0);

  uint srcId;
  alGenSources(1, &srcId);

  // Can this ever happen?
  if (srcId == INVALID_SOURCE) {
    soft_assert(false);

    alDeleteSources(1, &srcId);
    return INVALID_SOURCE;
  }

  if (alGetError() != AL_NO_ERROR) {
    // Too many sources.
    return INVALID_SOURCE;
  }

  alSourcei(srcId, AL_BUFFER, sounds[sound].handle);

  ++sounds[sound].nUsers;
  contSources.add(key, ContSource{ srcId, sound, true });
  return srcId;
}

void Context::removeContSource(ContSource* contSource, int key)
{
  int sound = contSource->sound;

  hard_assert(sounds[sound].nUsers > 0);

  alDeleteSources(1, &contSource->id);

  --sounds[sound].nUsers;
  contSources.exclude(key);
}

uint Context::requestSpeakSource(const char* text, int owner)
{
  if (espeak_Synth == nullptr || speakSource.thread.isValid()) {
    return INVALID_SOURCE;
  }

  speakSource.nQueuedBuffers = 0;
  speakSource.nSamples       = 0;
  speakSource.owner          = owner;
  speakSource.isAlive        = true;
  speakSource.text           = text;

  speakSource.thread = Thread("speak", speakMain);
  return speakSource.id;
}

void Context::releaseSpeakSource()
{
  hard_assert(speakSource.thread.isValid());

  speakSource.isAlive = false;
  speakSource.thread.join();
}

Context::Context() :
  imagoClasses(nullptr), audioClasses(nullptr), textures(nullptr), sounds(nullptr), models(nullptr),
  bspImagines(nullptr)
{}

Texture Context::loadTexture(const File& albedoFile, const File& masksFile,
                             const File& normalsFile)
{
  Texture texture;
  texture.id = -2;

  if (albedoFile.isFile()) {
    glGenTextures(1, &texture.albedo);
    glBindTexture(GL_TEXTURE_2D, texture.albedo);
    GL::textureDataFromFile(albedoFile, context.textureLod);
  }
  if (masksFile.isFile()) {
    glGenTextures(1, &texture.masks);
    glBindTexture(GL_TEXTURE_2D, texture.masks);
    GL::textureDataFromFile(masksFile, context.textureLod);
  }
  if (normalsFile.isFile()) {
    glGenTextures(1, &texture.normals);
    glBindTexture(GL_TEXTURE_2D, texture.normals);
    GL::textureDataFromFile(normalsFile, context.textureLod);
  }

  OZ_GL_CHECK_ERROR();
  return texture;
}

Texture Context::loadTexture(const String& basePath)
{
  return loadTexture(basePath + ".dds", basePath + "_m.dds", basePath + "_n.dds");
}

void Context::unloadTexture(const Texture* texture)
{
  if (texture->albedo != shader.defaultTexture) {
    glDeleteTextures(1, &texture->albedo);
  }
  if (texture->masks != shader.defaultMasks) {
    glDeleteTextures(1, &texture->masks);
  }
  if (texture->normals != shader.defaultNormals) {
    glDeleteTextures(1, &texture->normals);
  }

  OZ_GL_CHECK_ERROR();
}

Texture Context::requestTexture(int id)
{
  // No texture.
  if (id == -1) {
    return Texture();
  }

  Resource<Texture>& resource = textures[id];

  if (resource.nUsers >= 0) {
    ++resource.nUsers;
    return resource.handle;
  }

  const String& basePath = liber.textures[id].path;

  File albedoFile  = basePath + ".dds";
  File masksFile   = basePath + "_m.dds";
  File normalsFile = basePath + "_n.dds";

  resource.nUsers    = 1;
  resource.handle    = loadTexture(albedoFile, masksFile, normalsFile);
  resource.handle.id = id;

  return resource.handle;
}

void Context::releaseTexture(int id)
{
  // No texture.
  if (id == -1) {
    return;
  }

  Resource<Texture>& resource = textures[id];

  hard_assert(resource.nUsers > 0);

  --resource.nUsers;

  if (resource.nUsers == 0) {
    resource.nUsers = -1;
    unloadTexture(&resource.handle);
  }
}

uint Context::requestSound(int id)
{
  Resource<uint>& resource = sounds[id];

  if (resource.nUsers >= 0) {
    ++resource.nUsers;
    return resource.handle;
  }

  resource.nUsers = 1;

  OZ_AL_CHECK_ERROR();

  const String& name = liber.sounds[id].name;
  const File&   path = liber.sounds[id].path;

  alGenBuffers(1, &resource.handle);

  if (!AL::bufferDataFromFile(resource.handle, path)) {
    OZ_ERROR("Failed to load WAVE or Ogg Vorbis sound '%s'", name.c());
  }

  OZ_AL_CHECK_ERROR();
  return resource.handle;
}

void Context::releaseSound(int id)
{
  Resource<uint>& resource = sounds[id];

  hard_assert(resource.nUsers > 0);

  --resource.nUsers;

  if (resource.nUsers == 0) {
    alDeleteBuffers(1, &resource.handle);
    resource.nUsers = -1;
  }
}

void Context::playSample(int id)
{
  if (id < 0) {
    return;
  }

  uint srcId = addSource(id);

  if (srcId != INVALID_SOURCE) {
    alSourcei(srcId, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcePlay(srcId);
  }

  OZ_AL_CHECK_ERROR();
}

BSPImago* Context::getBSP(const BSP* bsp)
{
  Resource<BSPImago*>& resource = bspImagines[bsp->id];

  return resource.handle != nullptr && resource.handle->isLoaded() ? resource.handle : nullptr;
}

BSPImago* Context::requestBSP(const BSP* bsp)
{
  Resource<BSPImago*>& resource = bspImagines[bsp->id];

  // we don't count users, just to show there is at least one
  resource.nUsers = 1;

  if (resource.handle == nullptr) {
    resource.handle = new BSPImago(bsp);
  }

  return resource.handle;
}

void Context::drawBSP(const Struct* str)
{
  BSPImago* bsp = requestBSP(str->bsp);

  if (bsp->isLoaded()) {
    bsp->schedule(str, Model::SCENE_QUEUE);
  }
}

void Context::playBSP(const Struct* str)
{
  Resource<BSPAudio*>& resource = bspAudios[str->bsp->id];

  // we don't count users, just to show there is at least one
  resource.nUsers = 1;

  if (resource.handle == nullptr) {
    resource.handle = new BSPAudio(str->bsp);
  }

  resource.handle->play(str);
}

Model* Context::getModel(int id)
{
  Resource<Model*>& resource = models[id];

  return resource.handle != nullptr && resource.handle->isLoaded() ? resource.handle : nullptr;
}

Model* Context::requestModel(int id)
{
  Resource<Model*>& resource = models[id];

  if (resource.nUsers < 0) {
    resource.handle = new Model(liber.models[id].path);
    resource.nUsers = 1;
  }

  ++resource.nUsers;
  return resource.handle;
}

void Context::releaseModel(int id)
{
  Resource<Model*>& resource = models[id];

  hard_assert(resource.handle != nullptr && resource.nUsers > 0);

  --resource.nUsers;
}

PartClass* Context::getPartClass(int id)
{
  Resource<PartClass>& resource = partClasses[id];

  return &resource.handle;
}

PartClass* Context::requestPartClass(int id)
{
  Resource<PartClass>& resource = partClasses[id];

  if (resource.nUsers < 0) {
//    resource.handle.preload();
    resource.nUsers = 1;
  }

  ++resource.nUsers;
  return &resource.handle;
}

void Context::releasePartClass(int id)
{
  Resource<PartClass>& resource = partClasses[id];

  hard_assert(resource.nUsers > 0);

  --resource.nUsers;
}

void Context::drawImago(const Object* obj, const Imago* parent)
{
  hard_assert(obj->flags & Object::IMAGO_BIT);

  Imago* const* value = imagines.find(obj->index);

  if (value == nullptr) {
    Imago::CreateFunc* createFunc = imagoClasses[obj->clazz->imagoType];
    value = &imagines.add(obj->index, createFunc(obj)).value;
  }

  Imago* imago = *value;
  imago->flags |= Imago::UPDATED_BIT;
  imago->draw(parent);
}

void Context::playAudio(const Object* obj, const Object* parent)
{
  hard_assert(obj->flags & Object::AUDIO_BIT);

  Audio* const* value = audios.find(obj->index);

  if (value == nullptr) {
    Audio::CreateFunc* createFunc = audioClasses[obj->clazz->audioType];
    value = &audios.add(obj->index, createFunc(obj)).value;
  }

  Audio* audio = *value;
  audio->flags |= Audio::UPDATED_BIT;
  audio->play(parent);
}

void Context::drawFrag(const Frag* frag)
{
  FragPool* pool = fragPools[frag->poolId];

  if (pool == nullptr) {
    pool = new FragPool(frag->pool);
    fragPools[frag->poolId] = pool;
  }

  pool->flags |= FragPool::UPDATED_BIT;
  pool->draw(frag);
}

void Context::updateLoad()
{
  maxImagines           = max(maxImagines,           imagines.length());
  maxAudios             = max(maxAudios,             audios.length());
  maxSources            = max(maxSources,            Source::pool.length());
  maxContSources        = max(maxContSources,        contSources.length());
  maxPartGens           = max(maxPartGens,           partGens.length());

  maxSMMImagines        = max(maxSMMImagines,        SMMImago::pool.length());
  maxSMMVehicleImagines = max(maxSMMVehicleImagines, SMMVehicleImago::pool.length());
  maxExplosionImagines  = max(maxExplosionImagines,  ExplosionImago::pool.length());
  maxMD2Imagines        = max(maxMD2Imagines,        MD2Imago::pool.length());
  maxMD2WeaponImagines  = max(maxMD2WeaponImagines,  MD2WeaponImago::pool.length());

  maxBasicAudios        = max(maxBasicAudios,        BasicAudio::pool.length());
  maxBotAudios          = max(maxBotAudios,          BotAudio::pool.length());
  maxVehicleAudios      = max(maxVehicleAudios,      VehicleAudio::pool.length());
}

void Context::loadResources()
{
  if (!dynamicLoading) {
    for (int i = 0; i < liber.textures.length(); ++i) {
      requestTexture(i);
    }
    for (int i = 0; i < liber.sounds.length(); ++i) {
      requestSound(i);
    }
  }

  for (int i = 0; i < liber.models.length(); ++i) {
    models[i].handle = new Model(liber.models[i].path);
    models[i].handle->preload();
    models[i].handle->load();
    models[i].nUsers = 0;
  }

  for (int i = 0; i < liber.fragPools.length(); ++i) {
    fragPools[i] = new FragPool(liber.fragPools[i]);
  }

  for (int i = 0; i < liber.bsps.length(); ++i) {
    bspImagines[i].handle = new BSPImago(liber.bsps[i]);
    bspImagines[i].handle->preload();
    bspImagines[i].handle->load();
    bspImagines[i].nUsers = 0;

    bspAudios[i].handle = new BSPAudio(liber.bsps[i]);
    bspAudios[i].nUsers = 0;
  }

  OZ_GL_CHECK_ERROR();
  OZ_AL_CHECK_ERROR();
}

void Context::unloadResources()
{
  for (int i = 0; i < liber.bsps.length(); ++i) {
    delete bspImagines[i].handle;

    bspImagines[i].handle = nullptr;
    bspImagines[i].nUsers = -1;

    delete bspAudios[i].handle;

    bspAudios[i].handle = nullptr;
    bspAudios[i].nUsers = -1;
  }

  Arrays::free(fragPools, liber.fragPools.length());

  for (int i = 0; i < liber.models.length(); ++i) {
    delete models[i].handle;

    models[i].handle = nullptr;
    models[i].nUsers = -1;
  }
  Model::deallocate();

  if (!dynamicLoading) {
    for (int i = 0; i < liber.textures.length(); ++i) {
      releaseTexture(i);
    }
    for (int i = 0; i < liber.sounds.length(); ++i) {
      releaseSound(i);
    }
  }

  caelum.unload();
  terra.unload();

  OZ_GL_CHECK_ERROR();
  OZ_AL_CHECK_ERROR();
}

void Context::load()
{
  OZ_NACL_IS_MAIN(true);

  speakSource.owner = -1;

  alGenBuffers(2, speakSource.bufferIds);
  alGenSources(1, &speakSource.id);
  if (alGetError() != AL_NO_ERROR) {
    OZ_ERROR("Failed to create speak source");
  }

  maxImagines           = 0;
  maxAudios             = 0;
  maxSources            = 0;
  maxContSources        = 0;

  maxSMMImagines        = 0;
  maxSMMVehicleImagines = 0;
  maxExplosionImagines  = 0;
  maxMD2Imagines        = 0;
  maxMD2WeaponImagines  = 0;

  maxBasicAudios        = 0;
  maxBotAudios          = 0;
  maxVehicleAudios      = 0;

  imagines = HashMap<int, Imago*>(4096);
  audios   = HashMap<int, Audio*>(1024);

  if (!dynamicLoading) {
    loadResources();
  }
}

void Context::unload()
{
  OZ_NACL_IS_MAIN(true);

  Log::println("Unloading Context {");
  Log::indent();

  Log::println("Peak instances {");
  Log::indent();
  Log::println("%6d  imago objects",       maxImagines);
  Log::println("%6d  audio objects",       maxAudios);
  Log::println("%6d  one-time sources",    maxSources);
  Log::println("%6d  continuous sources",  maxContSources);
  Log::println("%6d  particle generators", maxPartGens);
  Log::println("%6d  SMM imagines",        maxSMMImagines);
  Log::println("%6d  SMMVehicle imagines", maxSMMVehicleImagines);
  Log::println("%6d  Explosion imagines",  maxExplosionImagines);
  Log::println("%6d  MD2 imagines",        maxMD2Imagines);
  Log::println("%6d  MD2Weapon imagines",  maxMD2WeaponImagines);
  Log::println("%6d  Basic audios",        maxBasicAudios);
  Log::println("%6d  Bot audios",          maxBotAudios);
  Log::println("%6d  Vehicle audios",      maxVehicleAudios);
  Log::unindent();
  Log::println("}");

  // Speak source must be destroyed before anything else using OpenAL since it calls OpenAL
  // functions from its own thread.
  if (speakSource.thread.isValid()) {
    releaseSpeakSource();
  }
  OZ_AL_CHECK_ERROR();

  alDeleteSources(1, &speakSource.id);
  alDeleteBuffers(2, speakSource.bufferIds);

  imagines.free();
  imagines.trim();
  audios.free();
  audios.trim();

  BasicAudio::pool.free();
  BotAudio::pool.free();
  VehicleAudio::pool.free();

  OZ_AL_CHECK_ERROR();

  SMMImago::pool.free();
  SMMVehicleImago::pool.free();
  ExplosionImago::pool.free();
  MD2Imago::pool.free();
  MD2WeaponImago::pool.free();

  while (!sources.isEmpty()) {
    removeSource(sources.first(), nullptr);
    OZ_AL_CHECK_ERROR();
  }
  for (auto i = contSources.iterator(); i.isValid();) {
    auto src = i;
    ++i;

    removeContSource(&src->value, src->key);
    OZ_AL_CHECK_ERROR();
  }

  sources.free();
  Source::pool.free();
  contSources.clear();
  contSources.trim();

  unloadResources();

  OZ_AL_CHECK_ERROR();

  Log::unindent();
  Log::println("}");
}

void Context::clearSounds()
{
  while (!sources.isEmpty()) {
    removeSource(sources.first(), nullptr);
    OZ_AL_CHECK_ERROR();
  }

  OZ_AL_CHECK_ERROR();
}

void Context::init()
{
  Log::print("Initialising Context ...");

  textureLod     = config.include("context.textureLod", 0).get(0);
  dynamicLoading = config.include("context.dynamicLoading", false).get(false);

  if (!liber.imagines.isEmpty()) {
    imagoClasses = new Imago::CreateFunc*[liber.imagines.length()] {};
  }
  if (!liber.audios.isEmpty()) {
    audioClasses = new Audio::CreateFunc*[liber.audios.length()] {};
  }
  if (!liber.fragPools.isEmpty() != 0) {
    fragPools = new FragPool*[liber.fragPools.length()] {};
  }

  OZ_REGISTER_IMAGOCLASS(SMM);
  OZ_REGISTER_IMAGOCLASS(SMMVehicle);
  OZ_REGISTER_IMAGOCLASS(Explosion);
  OZ_REGISTER_IMAGOCLASS(MD2);
  OZ_REGISTER_IMAGOCLASS(MD2Weapon);

  OZ_REGISTER_AUDIOCLASS(Basic);
  OZ_REGISTER_AUDIOCLASS(Bot);
  OZ_REGISTER_AUDIOCLASS(Vehicle);

  int nTextures    = liber.textures.length();
  int nSounds      = liber.sounds.length();
  int nPartClasses = liber.parts.length();
  int nModels      = liber.models.length();
  int nBSPs        = liber.bsps.length();

  textures    = nTextures    == 0 ? nullptr : new TextureResource[nTextures] {};
  sounds      = nSounds      == 0 ? nullptr : new SoundResource[nSounds] {};

  models      = nModels      == 0 ? nullptr : new Resource<Model*>[nModels] {};
  partClasses = nPartClasses == 0 ? nullptr : new Resource<PartClass>[nPartClasses] {};

  bspImagines = nBSPs        == 0 ? nullptr : new Resource<BSPImago*>[nBSPs] {};
  bspAudios   = nBSPs        == 0 ? nullptr : new Resource<BSPAudio*>[nBSPs] {};

  Log::printEnd(" OK");
}

void Context::destroy()
{
  Log::print("Destroying Context ...");

  delete[] imagoClasses;
  delete[] audioClasses;
  delete[] fragPools;

  delete[] textures;
  delete[] sounds;

  delete[] bspImagines;
  delete[] bspAudios;

  delete[] models;
  delete[] partClasses;

  imagoClasses = nullptr;
  audioClasses = nullptr;
  fragPools    = nullptr;

  textures     = nullptr;
  sounds       = nullptr;

  bspImagines  = nullptr;
  bspAudios    = nullptr;

  models       = nullptr;
  partClasses  = nullptr;

  Log::printEnd(" OK");
}

Context context;

}
}
