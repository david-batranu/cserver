SELECT uri,
       title,
       pubdate
FROM Articles
WHERE title LIKE '%:SearchString%'
  AND id NOT IN
    (SELECT id
     FROM Articles
     ORDER BY -pubdate
     LIMIT :PageOffset)
ORDER BY -pubdate
limit :PageSize;

