{ stdenv, fetchFromGitHub, mesa, glew, pkgconfig, openalSoft, freealut, wxGTK, libogg
, freetype, libvorbis, fftwSinglePrec, SDL, SDL_net, expat, libjpeg, libpng }:

stdenv.mkDerivation rec {
  version = "44";
  name = "scorched3d-${version}";
  src = ./.;

  buildInputs =
    [ mesa glew openalSoft freealut wxGTK libogg freetype libvorbis
      SDL SDL_net expat libjpeg libpng fftwSinglePrec
    ];

  nativeBuildInputs = [ pkgconfig ];

  sourceRoot = "scorched";

  configureFlags = [ "--with-fftw=${fftwSinglePrec.dev}" ];

  NIX_LDFLAGS = [ "-lopenal" ];

  meta = with stdenv.lib; {
    homepage = http://scorched3d.co.uk/;
    description = "3D Clone of the classic Scorched Earth";
    license = licenses.gpl2Plus;
    platforms = platforms.linux;
  };
}
