import sqlite3
import feedparser
import dateparser
import datetime
import time
import pytz

DB = "file:../main.db?mode=rw"


con = sqlite3.connect(DB, uri=True)
cur = con.cursor()

sources = cur.execute("SELECT * FROM Sources;").fetchall()


def datetime_now():
    return pytz.utc.localize(datetime.datetime.utcnow())


for _id, uri, db_title in sources:
    d = feedparser.parse(uri)
    title = d.feed.get("title", uri)
    if title != db_title:
        cur.execute(
            "UPDATE Sources set title = ? WHERE id == ?",
            (
                title,
                _id,
            ),
        )
        con.commit()

    for item in d.entries:
        uri = item.id
        exists = cur.execute("SELECT id FROM Sources WHERE uri == ?", (uri,)).fetchone()

        if exists:
            continue

        title = item.title
        description = item.description
        published = time.mktime(
            item.get("published_parsed")
            or dateparser.parse(item["published"].split(",")[-1]).timetuple(),
        )
        created = time.mktime(datetime_now().timetuple())

        cur.execute(
            (
                "INSERT INTO Articles(id, uri, title, description, pubdate, date, sourceid)"
                " values(NULL, ?, ?, ?, ?, ?, ?);"
            ),
            (
                uri,
                title,
                description,
                int(published),
                int(created),
                _id,
            ),
        )

    con.commit()
