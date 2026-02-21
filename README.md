# Linkwitz-Riley-Saturator (Fork Readme)
Version 1.0.0 attempts to respect the original design, bringing it to completion with code refinements and the creation of a GUI (but the central rotary switch is too small).
Version 1.1.0 implements a 4th-order Linkwitz-Riley filter (LR4 at 24 dB/octave). Unlike the LR2, the LR4 has a much steeper slope (better protecting the drivers and isolating the bands) but maintains the same polarity between the channels without having to invert the phase.
Instead of a single filter per band, there are two cascaded 2nd-order (Butterworth) filters. Mathematically: $LR4 = (Butterworth 2)^2$.
Constant Phase: With the LR4, the sum at the crossover point is perfectly flat (0 dB) and the signals are in phase (0° difference), eliminating the need for the 0.707 correction factor used in the first version.
Intelligent Saturation: Saturation only on the low band, but with a sharper cutoff thanks to the 24 dB/octave.

# Linkwitz-Riley-Saturator (original repo Readme)
2 Band Linkwitz-Riley Filter, with Tube Saturation applied to the lower band
