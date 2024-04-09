Creates QVariantMap from ini.
Supports mulitline value (use <T%> as start and <%T> on newline as end , use without braces)
Structure:
QMap(INI,QVariantMap(SECTION,QVariantMap(KEY,VALUE)))
