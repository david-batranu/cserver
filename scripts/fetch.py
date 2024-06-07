import logging
import sqlite3
import feedparser
import dateparser
import datetime
import time
import pytz

logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

log = logging.getLogger("fetch-articles")

DB = "file:../main.db?mode=rw"


con = sqlite3.connect(DB, uri=True)
cur = con.cursor()

sources = cur.execute("SELECT * FROM Sources;").fetchall()


def datetime_now():
    return pytz.utc.localize(datetime.datetime.utcnow())


for _id, uri, db_title in sources:
    d = feedparser.parse(uri)
    title = d.feed.get("title", uri)
    log.info("Fetching %s...", uri)
    if title != db_title:
        cur.execute(
            "UPDATE Sources set title = ? WHERE id == ?",
            (
                title,
                _id,
            ),
        )
        con.commit()

    items_to_add = []
    for item in d.entries:
        uri = item.id
        exists = cur.execute("SELECT id FROM Articles WHERE uri == ?", (uri,)).fetchone()

        if exists:
            continue

        log.info("Adding new article: %s...", uri)
        title = item.title
        description = item.description
        published = time.mktime(
            item.get("published_parsed")
            or dateparser.parse(item["published"].split(",")[-1]).timetuple(),
        )
        created = time.mktime(datetime_now().timetuple())
        items_to_add.append((
            uri,
            title,
            description,
            int(published),
            int(created),
            _id,
        ))

    cur.executemany(
        (
            "INSERT INTO Articles(id, uri, title, description, pubdate, date, sourceid)"
            " values(NULL, ?, ?, ?, ?, ?, ?);"
        ),
        items_to_add
    )

    con.commit()
