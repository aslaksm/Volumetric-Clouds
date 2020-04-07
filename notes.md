Notes from the pixar paper
- https://graphics.pixar.com/library/ProductionVolumeRendering/paper.pdf

Talking about volumetric rendering. What a volume means, what are its properties.
The properties of volumes are defined by absorption and scattering coefficients, together with the phase function and emission.

**Absorption** is when a photon collides and is absorbed by the volume. In this case, the photon simply disappears (transformed into heat).

A **scattering collision** scatter the photon in a different direction, defined by the *phase function*.

The **phase function** is the angular distribution of radiance scattered. Usually modeled as a 1D function of the angle theta between the two directions w and w'. They need to be normalized over the sphere (phase function integrated over sphere = 1), otherwise the function would add of subtract radiance to a scattering collision.
The most widely used phase function is the Henyey-Greenstein phase function.

**Emission**. Volumes emit radiance in a volumetric fashion, but otherwise behave exactly like any other light source. The emitted radiance is expressed as a radiance field whose output gets absorbed and scattered like any non-volume light source.

We can define an extinction coefficient `ot = oa + os`, the sum of an absorption and scattering coefficient. This is sometimes called the attenuation coefficient.
Extinction defines the net loss of radiance due to both absorption and scattering.

