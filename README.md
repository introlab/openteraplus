<img src="docs/logo/LogoOpenTeraPlus.png" width="500">

# OpenTeraPlus
OpenTeraPlus is a client that works with [OpenTera Server](https://github.com/introlab/opentera). It aims to act as a general tool to manage an OpenTera instance and to provide service specific features, such as VideoRehabilitation.

## Authors

* Simon Brière, ing. M.Sc.A., Research Center on Aging, CIUSSS de l'Estrie-CHUS (@sbriere)
* Dominic Létourneau, ing. M.Sc.A., IntRoLab, Université de Sherbrooke (@doumdi)
* François Michaud, ing. Ph.D., IntRoLab, Université de Sherbrooke
* Michel Tousignant, pht, Ph.D., CDRV, Université de Sherbrooke

## Publication(s)

* [![DOI](https://joss.theoj.org/papers/10.21105/joss.05497/status.svg)](https://doi.org/10.21105/joss.05497) Létourneau, D., Brière , S.,  et al., [OpenTera: A Framework for Telehealth Applications](https://doi.org/10.21105/joss.05497), Journal of Open Source Software, vol. 8, no 91, p. 5497 (2023)
* Panchea, A.M., Létourneau, D., Brière, S. et al., [OpenTera: A microservice architecture solution for rapid prototyping of robotic solutions to COVID-19 challenges in care facilities](https://rdcu.be/cHzmf),  Health Technol. 12, 583–596 (2022)

## Current Features

### High-level features
* Global system management including users access roles for specific sites and projects, devices, sessions types and services
* Participant (patient) management, including activation/deactivation, grouping and sessions dashboard
* Specific integration for telerehabilitation services and all clinical activites that involves a secure and adapted audio-video-data session
* Sessions allows to have data files attached to them and events

### Detailled features
* User-based login
  * Implementation of role access architecture from OpenTera Server
* Site and project management
  * User groups access
  * Session types
  * Allowed services
* Participant (patient) management
  * Participant grouping
  * Participant enabling / disabling, including web access and traditional password
* Session management
  * New session creation and edition of existing sessions
  * Calendar view of realized and planned sessions
  * Session events view
  * Assets (data file) download and management
* Devices management
  * Device association
  * Device enabling / disabling
  * Device configuration editing
* Services managements
  * Edit configuration
* Assets managements
  * View and download assets related to a session
* Video / tele rehabilitation sessions
  * Create and manage video sessions, with a focus on telerehabilitation 
* ... and more!

# Screenshots
<table><tr>
<td><img src="docs/screenshots/LoginScreen.png" width="100" alt="Login Screen" title="Login Screen"></td>
<td><img src="docs/screenshots/ProjectNavigator.png" width="100" alt="Project Navigator" title="Project Navigator"></td>
<td><img src="docs/screenshots/Participant.png" width="100" alt="Participant Viewer" title="Participant Viewer"></td>
<td><img src="docs/screenshots/SessionLobby.png" width="100" alt="Session Lobby" title="Session Lobby"></td>
<td><img src="docs/screenshots/AdminConfig.png" width="100" alt="Admin configuration" title="Admin configuration"></td>
</tr></table>

# Video
[![OpenTera+ Clinical Telehealth Software](https://img.youtube.com/vi/4YMKSUE6xJs/maxresdefault.jpg)](https://youtu.be/4YMKSUE6xJs)
 
# License
OpenTeraPlus is licensed under [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)
