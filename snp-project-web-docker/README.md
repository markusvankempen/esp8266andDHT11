# BOOTSTRAP CSS + NGINX + DOCKER

Simple Nginx server docker container running a static Bootstrap template
https://startbootstrap.com/theme/grayscale

## How to run

You will need Docker on your machine.

```bash
# Build Image
docker build.  -t snp-webpage

# Run container
docker run -d -p 80:80 snp-webpage

# Confirm container is running
docker container ls
#
docker login 

#tag
docker images
docker tag 9241424084be username/snp-webserver:second
#push to dockerhub
docker push username/snp-webserver:second
```

Open your browser at localhost or specify public DNS or IP address to confirm that the web server is running.
