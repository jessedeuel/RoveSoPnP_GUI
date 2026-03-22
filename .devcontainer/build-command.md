docker buildx build --platform linux/arm64,linux/amd64 -t jessedeuel/rovesopnp_gui:latest --push --progress plain --build-arg QT_EMAIL --build-arg QT_PASSWD -f Ubuntu.dockerfile .
