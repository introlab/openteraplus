#include "TeraAsset.h"

TeraAsset::TeraAsset(QObject *parent)
    : QObject{parent}
{

}

QString TeraAsset::getIconForContentType(const QString &content_type)
{
    // Get icon for the specific content-type
//    QMimeDatabase mime_db;
//    QMimeType mime_type = mime_db.mimeTypeForName(content_type);

//    return mime_type.genericIconName();

    if (content_type.endsWith("octet-stream"))
        return "://icons/assets/binary.png";

    if (content_type.startsWith("video"))
        return "://icons/assets/video.png";

    if (content_type.startsWith("audio"))
        return "://icons/assets/audio.png";

    if (content_type.endsWith("database") || content_type.endsWith("sql"))
        return "://icons/assets/database.png";

    if (content_type.contains("graph"))
        return "://icons/assets/graph.png";

    if (content_type.endsWith("json"))
        return "://icons/assets/json.png";

    if (content_type.endsWith("pdf"))
        return "://icons/assets/pdf.png";

    if (content_type.startsWith("text") || content_type.contains("document"))
        return "://icons/assets/document.png";

    if (content_type.contains("zip"))
        return "://icons/assets/zip.png";

    if (content_type.startsWith("image"))
        return "://icons/assets/image.png";

    return "://icons/assets/unknown.png";
}
