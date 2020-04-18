I had a great deal of difficulty figuring out the lighting model.

I used Andrew Schneider's GPU PRO 7 article as a basis for a lot of the work


There are 3 essential elements to model:

- Transmittance
- In-scattering
- Forward-scattering


We march toward the light in a cone, taking 6 samples, as well as 1 additional far-away sample which we use to calculate clouds (bit of a hail-mary).

The cloud itself is the participating medium. As a light ray travels through the medium, some of the light will be diminished (more on this later). The ability of the medium to transmit light without diminishment is referred to as its *transmittance*. The transmittance ranges from 0.0 to 1.0, with 0.0 indicating that no light makes it through the medium.
Intuitively, the 

I encountered a lot of problems underway. In the beginning, I expected the base task to be fairly simple, as there were a lot of helpful resources and articles on the subject. However, a lot of crucial aspects are often ignored or glossed over. I spent quite a bit of time trying to figure out how not to sample the clouds so that a layering effect occured.

[Insert video of layering clouds]

I managed to get decent results by adding a random offset in the range [0, STEP_SIZE], but this introduced serious noise artifacts. I ended up using a Bayer Filter to calculate the offset, an approach I saw used in various implementations. This results noticeable artifacts, in particular when using a low number of steps. For steps > 20 however, the artifacts are negligible.

Since the clouds are far above the origin, I needed some way of calculating where the beginning and end of the skybox should be. I ended up calculating two sphere intersections, one for the inner cloud shell and one for the outer. The sphere origin is set to be the center of the earth, with the assumption that the origin is placed on the ground. This intersection is calculated for every raymarch. This approach also effectively mimics the effect of clouds vanishing into the horizon. In order avoid rendering clouds below the horizon, we only render clouds when the view-ray is positive in the y-direction.

I had a LOT of issues with the lighting model. The lightmarch takes 6 samples towards the sun. We move towards the sun in a cone in order to reduce artifacts.

I had to do a lot of experimenting and tweaking with parameters in order to get something that vaguely resembled clouds.

The first step was to simply use the transmittance value of the clouds to determine cloud opacity.

[Picture of transmittance opacity]

After this, I added color based on the transmittance of the light samples at each step. This was scaled by step_size / total_length in order to have similar coloring independent of the no. of samples.

[Picture of light transmittance coloring]


I had a lot of trouble modeling in-scattering. Andrew Schneider's formula, `1.0 - exp(- light_density * 2)`, didn't work at all.

[Picture of shit inScattering]

NadirRogue uses a slightly different version in his engine, which I tried as well. The results look decent when the base cloud is scaled far out, but for any greater values a lot of artifacts arise. For this reason I decided to skip in-scattering.


I had more luck with forward scattering (silver lining effect). however, it absolutely didn't work right out the gate. It initially almost completely saturated the light, and it was only after a lot of tweaking that I got something decent-looking.

[Picture of forward scattering]


Refinements

I did a lot of tooling around with the noies. However, at some point I had to implement tiling Perlin noise: At this point I was pretty tired of the project, so I just used Sebastien Hillare's TilableVolumeNoise.

Fixing coverage and base noise.
