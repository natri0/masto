# Unnamed MastoAPI Client

This is an app I use for shitposting on Mastodon — loads quick, has a simple UI and posts quick.

## Features

1. **Text posts.** Type and hit <kbd>Ctrl+Enter</kbd> to post.
2. **Configurable.** Set your instance and window size (*todo: default visibility*).
3. **Fast.** Loads in a second, posts in a second, quits in a second.

## Wanted features

(Not in any particular order)

- [ ] Attachments via drag-and-drop
- [ ] CWs
- [ ] Multiple accounts
- [ ] Thread mode
- [ ] Non-Linux OS support

## Building

1. Clone the repo *with submodules* (`git clone --recursive`)
2. Make sure you have `libdbus-1` and `libcurl` installed
3. Generate buildscript with CMake: `cmake -G Ninja -B build .`
4. Build: `ninja -C build`

## Thanks

- [Mastodon API](https://docs.joinmastodon.org/api/)
- [cURL](https://curl.se/)
- [raylib](https://raylib.com/)
- [nlohmann/json](https://github.com/nlohmann/json)
