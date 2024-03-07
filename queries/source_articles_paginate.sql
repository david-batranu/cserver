SELECT uri,
       title,
       pubdate
FROM Articles
WHERE sourceid = :SourceID
  AND id NOT IN
    (SELECT id
     FROM Articles
     WHERE sourceid = :SourceID
     ORDER BY -pubdate
     LIMIT :PageOffset)
ORDER BY -pubdate
limit :PageSize;

