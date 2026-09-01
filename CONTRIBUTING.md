# Contributing to Black Hole Lab

Thank you for your interest in contributing to Black Hole Lab! We welcome all forms of contributions, from bug reports to enhancements and optimizations.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Install dependencies** (GLFW via Homebrew on macOS)
4. **Build the project** using the command in `README.md`
5. **Create a feature branch** from `main`

## Development Workflow

- Keep changes focused and atomic
- Write clear commit messages
- Test your changes before submitting a pull request
- Follow the existing code style (C++17 standards)

## Types of Contributions

### Bug Fixes
- Report issues in the GitHub Issues section
- Include steps to reproduce and your system specs
- Submit a pull request with a clear fix

### Enhancements
- Physics accuracy improvements (ray-tracing, geodesics, etc.)
- Performance optimizations (especially for Retina displays)
- UI/UX improvements
- Documentation clarifications

### Graphics & Visualization
- Better accretion disk models
- Improved jet rendering
- Bloom or tone-mapping refinements
- Additional black hole presets

## Testing Your Changes

Before submitting a PR:

```bash
clang++ -std=c++17 -O2 -Wall -Wextra \
  -I/opt/homebrew/include -I/usr/local/include \
  ZLATAN.cpp -o ZLATAN \
  -L/opt/homebrew/lib -L/usr/local/lib \
  -lglfw -framework OpenGL

./ZLATAN
```

Verify that the application runs smoothly and visually renders correctly.

## Code Style

- Use consistent indentation (spaces preferred)
- Keep lines reasonably length
- Comment non-obvious physics or algorithm choices
- Follow C++17 conventions

## Pull Request Process

1. Update the README if you've added new features or controls
2. Write a clear PR title and description
3. Reference any related issues
4. Ensure your changes compile without warnings
5. Wait for feedback and be open to suggestions

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

Thank you for helping make Black Hole Lab better! 🚀
