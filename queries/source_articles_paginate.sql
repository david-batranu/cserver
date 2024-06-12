SELECT uri,
       title,
       pubdate
FROM Articles
WHERE sourceid = :SourceID
ORDER BY pubdate DESC
limit :PageOffset, :PageSize;

