s|MOV result.color, r0;|\
PARAM p = program.env[8];\
TEMP fogFactor;\
MUL fogFactor.x, p.x, fragment.fogcoord.x;\
EX2_SAT fogFactor.x, -fogFactor.x;\
MUL fogFactor.y, p.y, fragment.fogcoord.x;\
MUL fogFactor.y, fogFactor.y, fogFactor.y;\
EX2_SAT fogFactor.y, -fogFactor.y;\
MAD_SAT fogFactor.z, p.z, fragment.fogcoord.x, p.w;\
MUL fogFactor.x, fogFactor.y, fogFactor.x;\
MUL fogFactor.x, fogFactor.z, fogFactor.x;\
LRP r0.rgb, fogFactor.x, r0, state.fog.color;\
MOV result.color, r0;\
|g
